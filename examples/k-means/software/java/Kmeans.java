// Copyright 2018 Delft University of Technology
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

package nl.tudelft.ewi.ce.abs.kmeans;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

public class Kmeans {
	int num_columns = 8;
	int num_centroids = 16;
	int iteration_limit = 25;
	int chunk_size = 8192;

	
	public static void main(String[] args) throws InterruptedException, ExecutionException {
		Timer t = new Timer();
		
		long num_rows = 1024*1024*1024/(64/8)/8;  // For 1 GiB of data
		int ne = 10;  // Number of experiments
		
		Kmeans kmeans = new Kmeans();
		
		DatasetGenerator generator = new ClusteredDatasetGenerator(kmeans.num_columns, kmeans.num_centroids, num_rows);
		List<List<Long>> data = generator.getDataset();
		
		List<List<Long>> start_centroids = kmeans.getStartingCentroids(data);
		
		
		double t_vcpu = 0;
		double t_vomp = 0;
		for (int experiment = 0; experiment < ne; experiment++) {
		
			t.start();
			List<List<Long>> result_vcpu = kmeans.kmeansCPU(data, start_centroids);
			t.stop();
			t_vcpu += t.seconds();
			
			System.out.println("vCPU clusters: ");
			if (experiment == 0) {
				for (List<Long> centroid : result_vcpu) {
					System.out.println(centroid);
				}
			}
			
			t.start();
			List<List<Long>> result_vomp = kmeans.kmeansCPUThreaded(data, start_centroids);
			t.stop();
			t_vomp += t.seconds();
			if (!result_vomp.equals(result_vcpu)) {
				System.out.println("ERROR");
				System.exit(1);
			}
		}
		
		System.out.println(t_vcpu + " vCPU");
		System.out.println(t_vomp + " vOMP");
		
		System.out.println("PASS");
		System.exit(0);
	}

	
	private List<List<Long>> getStartingCentroids(List<List<Long>> data) {
		List<List<Long>> centroids = new ArrayList<List<Long>>(num_centroids);
		for (int c = 0; c < num_centroids; c++) {
			centroids.add(data.get(c));
		}
		return centroidsCopy(centroids);
	}
	
	
	private List<List<Long>> centroidsCopy(List<List<Long>> centroids) {
		List<List<Long>> new_centroids = new ArrayList<List<Long>>(num_centroids);
		for (List<Long> row : centroids) {
			new_centroids.add(new ArrayList<Long>(row));
		}
		return new_centroids;
	}
	
	
	private List<List<Long>> kmeansCPU(List<List<Long>> data, List<List<Long>> centroids) {
		long[] counters = new long[num_centroids];
		long[][] accumulators = new long[num_centroids][num_columns];
		
		List<List<Long>> old_centroids;
		List<List<Long>> new_centroids = centroidsCopy(centroids);
		int iteration = 0;
		do {
			old_centroids = centroidsCopy(new_centroids);

			// For each point
			for (List<Long> row : data) {
				// Determine closest centroid for point
				int closest = 0;
				long min_distance = Long.MAX_VALUE;
				for (int c = 0; c < num_centroids; c++) {
					List<Long> centroid = old_centroids.get(c);
					long distance = 0;
					for (int d = 0; d < num_columns; d++) {
						long dim_distance = row.get(d) - centroid.get(d);
						distance += dim_distance * dim_distance;
					}
					if (distance <= min_distance) {
						closest = c;
						min_distance = distance;
					}
				}
				// Update counters of closest centroid
				counters[closest]++;
				for (int d = 0; d < num_columns; d++) {
					accumulators[closest][d] += row.get(d);
				}
			}
			
			// Calculate new centroid positions
			for (int c = 0; c < num_centroids; c++) {
				List<Long> centroid = new_centroids.get(c);
				for (int d = 0; d < num_columns; d++) {
					centroid.set(d, accumulators[c][d] / counters[c]);
					accumulators[c][d] = 0;
				}
				counters[c] = 0;
			}
			System.err.println("iteration " + iteration);
			iteration++;
		} while (!old_centroids.equals(new_centroids) && iteration < iteration_limit);

		return new_centroids;
	}
	
	
	private List<List<Long>> kmeansCPUThreaded(List<List<Long>> data, List<List<Long>> centroids) throws InterruptedException, ExecutionException {
		ExecutorService es = Executors.newWorkStealingPool();
		
		long[] counters = new long[num_centroids];
		long[][] accumulators = new long[num_centroids][num_columns];
		
		List<List<Long>> old_centroids;
		List<List<Long>> new_centroids = centroidsCopy(centroids);
		int iteration = 0;
		do {
			old_centroids = centroidsCopy(new_centroids);

			// Split points over work units
			List<Future<KmeansPartialResult>> results = new ArrayList<Future<KmeansPartialResult>>();
			int n = 0;
			for (; n < data.size() - chunk_size; n += chunk_size) {
				results.add(es.submit(new KmeansAssigner(this, new_centroids, data.subList(n, n + chunk_size))));
			}
			results.add(es.submit(new KmeansAssigner(this, new_centroids, data.subList(n, data.size()))));
			
			// Add all results together
			for (Future<KmeansPartialResult> result : results) {
				KmeansPartialResult vals = result.get();
				for (int c = 0; c < num_centroids; c++) {
					for (int d = 0; d < num_columns; d++) {
						accumulators[c][d] += vals.accumulators[c][d];
					}
					counters[c] += vals.counters[c];
				}
			}
			
			// Calculate new centroid positions
			for (int c = 0; c < num_centroids; c++) {
				List<Long> centroid = new_centroids.get(c);
				for (int d = 0; d < num_columns; d++) {
					centroid.set(d, accumulators[c][d] / counters[c]);
					accumulators[c][d] = 0;
				}
				counters[c] = 0;
			}

			iteration++;
		} while (!old_centroids.equals(new_centroids) && iteration < iteration_limit);

		return new_centroids;
	}
}
