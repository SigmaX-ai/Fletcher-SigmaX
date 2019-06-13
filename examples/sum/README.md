# Getting Started: Simple Column Sum Example

The main goal of this tutorial is to learn the basics of working with the Fletcher framework.

To this end, let's assume we want to create a very simple FPGA accelerator design that just sums a column of integers 
in a table, or in Apache Arrow terminology: sums an array (column) of integers of some recordbatch (tabular data 
structure).

Schematically, our desired functionality would look as follows:

![](doc/sum_top.svg)

We've included some dummy data in our ExampleBatch "table" that we'll also use throughout this example. 

The next few steps will take us through how we can use Fletcher to realize the functionality shown above.

1. [Generate a Schema](#1-generate-a-schema) to represent the type of RecordBatch the kernel must operate on.

2. [Generate a RecordBatch](#2-generate-a-recordbatch) with some dummy data for simulation.

3. [Run Fletchgen](#3-run-fletchgen) to create a design and kernel template from the Schema and RecordBatch.

4. [Implement the kernel](#4-implement-the-kernel).

5. [Simulate the design.](#5-simulate-the-design)

# 1. Generate a Schema

Fletchgen is the design generator tool of Fletcher. It requires an Arrow *schema* as input. A schema is nothing more 
than a description of the type of data that you can find in an Arrow *recordbatch*. A recordbatch is a tabular 
datastructure with columns. These columns are called Arrow *arrays*. Don't confuse them with C-like arrays! 
They can be much more complex than that!

When we look at the figure above, we see that our computational kernel (the Sum kernel) will just take one column from
one RecordBatch, and spit out the sum of all the numbers in the column. 

We observe that name of the column with the numbers is `number`. Suppose we'll choose the numbers to be of Arrow type 
`int64`. We may now describe an Arrow *field* of the schema that corresponds to our `number` column. 

There is one more thing that Arrow needs to know about the field; whether or not it's *nullable*, meaning that the 
number can also be invalid (for example, there was a corrupted sample in some measurement).  For now, we'll assume that
the numbers are not nullable (i.e. they are always valid).

We will use a little Python script to generate the field. 

[Continue the tutorial by checking out this iPython Notebook.](hardware/generate-schema.ipynb)

If you're in a hurry, [here is the short version, that includes step 2.](hardware/generate-input.py)

# 2. Generate a RecordBatch

Now that we've defined an Arrow schema, we can move to the next step of our typical design flow. Not only do we
want to design the kernel based on the schema, we'd also like to feed it with some data in a simulation environment,
before throwing our design at the FPGA place and route tools (this can take multiple hours to complete!).

To this end, we can supply Fletchgen with an actual Arrow recordbatch. In real applications, these can be very large;
in the order of gigabytes. For simulation we're just going to fill our only recordbatch column with four numbers as
is shown in the figure at the top of this page.

We'll also use a little Python script to generate the recordbatch.

[Continue the tutorial by checking out this iPython Notebook.](hardware/generate-recordbatch.ipynb)

If you're in a hurry, [here is the short version, that includes step 1.](hardware/generate-input.py)

# 3. Run Fletchgen

Now that we've generated a recordbatch with some test data, it's time to let Fletchgen do its job.
We want Fletchgen to do the following things for us:

1. Create a kernel template based on the schema.
2. Create a design infrastructure based on the schema.
3. Create a simulation top-level with pre-loaded memory model contents based on the recordbatch.

We can call fletchgen from the command line as follow:

```console
$ cd hardware
$ fletchgen -n Sum -r input/recordbatch.rb -s output/recordbatch.srec -l vhdl --sim
```

We've used the following options:
* `-n` specifies the name of our kernel.
* `-r` specifies that we want to give Fletchgen a recordbatch.
* `-s` specifies the path of the memory model contents (an SREC file)
* `-l` specifies the output language for the design files, in our case VHDL (I know right?)
* `--sim` specifies that we'd like to generate a simulation top-level.

Now, Fletchgen will generate all the things we've listed above. The output files are as follows:

* `hardware/output/recordbatch.srec`: our memory model contents. 
* `hardware/vhdl/Sum.vhdt`: a template for the Sum kernel. Note that there was already a `Sum.vhd` file in that folder,
so Fletchgen was kind enough not to overwrite it (we've already implemented the kernel there). 
It will always overwrite a `.vhdt` file, though!
* `hardware/vhdl/ExampleBatch.vhd`: a generated "RecordBatchReader".
* `hardware/vhdl/Mantle.vhd`: a generated top-level wrapper instantiating the "RecordBatchReader" and the "Sum" kernel.
* `hardware/vhdl/sim_top.vhd`: the simulation top-level, instantiating the memory models and the "Mantle".

# 4. Implement the kernel

# 5. Simulate the design

## Generating the hardware skeleton files

Note: The produced files are available in the repository, so these steps
can be skipped if so desired. They also provide a reference for when
the tools are updated and their output changes. The goal is to track these
changes into this example as quickly as possible, but it may lag behind.

The main component of Fletcher is the
[ArrayReader](../../hardware/vhdl/arrays/ArrayReader.vhd).
The array reader will transparently read rows of data from memory and
provide them on its output. Multiple array readers are supported with the
use of an [arbiter](../../hardware/vhdl/interconnect/BusReadArbiterVec.vhd),
though only one reader is used in this example.
Next to this, a controller is added, which communicates with the host software
and can be used to coordinate work in more complex designs.
A default controller is provided.
These components, together with the user provided accelerator, are combined
into one component named *fletcher_wrapper*.

The *fletcher_wrapper* component is platform independent. To communicate
with target platform host software, a host-specific interface must be added.
The [AWS F1](https://github.com/aws/aws-fpga)
and [CAPI SNAP](https://github.com/open-power/snap)
platforms use memory mapped IO and an AXI bus.
For these platforms, an interface layer is added that converts the
Fletcher bus requests to AXI format.
For configuration and control of the hardware function from the
host software, [MMIO registers](../../hardware/axi/axi_mmio.vhd) are used.

The above will be contained in the following files:

  * [sum.vhd](./hardware/axi_top.vhd)
Hardware accelerated function, provided by the user
  * [fletcher_wrapper.vhd](./hardware/fletcher_wrapper.vhd)
Readers and associated hardware (Fletcher)
  * [axi_top.vhd](./hardware/axi_top.vhd)
Interface for hosts using AXI.

The core accelerator function will be implemented inside the generated
template and will differ depending on the Arrow schema that is used
for storing the data.
The instantiation template for the accelerated function can be generated
automatically from an Arrow schema by [fletchgen](../../codegen/fletchgen/).

### Requirements
  * C++11 compatible compiler (tested with GCC 5.5)
  * Boost development libraries
      * system
      * filesystem
      * regex
      * program_options
  * A VHDL simulator (e.g. ModelSim)

### Install Apache Arrow

Install Apache Arrow. The supported release is
[0.10.0](https://github.com/apache/arrow/releases/tag/apache-arrow-0.10.0).

    $ curl "https://codeload.github.com/apache/arrow/tar.gz/apache-arrow-0.10.0" \
        | tar --extract --gzip
    $ cd arrow-apache-arrow-0.10.0
    $ mkdir cpp/build
    $ cd cpp/build
    $ cmake \
        -DARROW_PLASMA=ON \
        ..
    $ make install

### Get Fletcher

Clone the Fletcher repository and set up the environment.

    $ git clone https://github.com/johanpel/fletcher.git
    $ cd fletcher

### Build fletchgen

Build [fletchgen](../../codegen/fletchgen/) as described in its README.

    $ cd codegen/fletchgen
    $ mkdir debug
    $ cd debug
    $ cmake ..
    $ make install

### Create a schema file

The schema that is used for the input data must be written to a file
in order for fletchgen to use it.
An example of how to generate such a file using Apache Arrow can be found
in [schema.cpp](./hardware-gen/schema.cpp).
The provided CMake project will create the schema file and the
hardware skeleton files.

    $ cd examples/sum/hardware-gen"
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make

To create a schema manually after compiling:

    $ ./schema

### Generate hardware skeleton files

With the schema file (*sum.fbs*) from the previous step,
generate the skeleton files.
The files will already have been generated if you used the CMake project in
the previous step. This will produce fletcher_wrapper.vhd and axi_top.vhd.

To run fletchgen manually:

    $ fletchgen --quiet \
        --input sum.fbs \
        --output fletcher_wrapper.vhd \
        --axi axi_top.vhd

## Implement accelerated function

Design the hardware accelerated function, using the component description of
*sum* in the generated [fletcher_wrapper.vhd](./hardware/fletcher_wrapper.vhd).
The design will need a simple state machine, control signals,
and status signals.
The finished design is included ([hardware/sum.vhd](./hardware/sum.vhd))
in the repository.

## Create testbench

TODO: Generate SREC file and run platform-independent simulation.

## Run on hardware

If the tests pass, pick a supported platform to synthesise the design for.
An [example](../../platforms/aws-f1/examples/sum) exist for AWS F1 instances.
