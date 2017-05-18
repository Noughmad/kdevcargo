# KDevelop plugin with support for Cargo

[![Codefresh build status]( https://g.codefresh.io/api/badges/build?repoOwner=Noughmad&repoName=kdevcargo&branch=master&pipelineName=kdevcargo&accountName=Noughmad&type=cf-1)]( https://g.codefresh.io/repositories/Noughmad/kdevcargo/builds?filter=trigger:build;branch:master;service:591d7e9809f4a600015ecd49~kdevcargo)

This plugin enables the use of Cargo to manage, build and run Rust packages (crates) with Cargo.
It recognizes `Cargo.toml` files as project files, builds them using `cargo build`, and allows easy run configuration using `cargo run`.


## Installation instructions

This plugin supports KDevelop 5, so make sure have kdevplatform libraries development headers (`kdevplatform-dev` on ubuntu) version 5 or later.
You also need `cmake` and `extra-cmake-modules` to build it.
To use the plugin, the `cargo` executable must be installed and in your `PATH`.

Download this repository, then run

    mkdir build && cmake .. && make
    sudo make install

This installs to a system directory, most likely `/usr/local`, but it will not be picked up by KDevelop.
To make KDevelop load this plugin, set the `QT_PLUGIN_PATH` environment variable to point to its install location.
If it was installed to `/usr/local/lib64/plugins/kdevplatform/27/kdevcargo.so`, run from a console

    QT_PLUGIN_PATH=${QT_PLUGIN_PATH}:/usr/local/lib64/plugins kdevelop

If everything went well, you should see the Cargo plugin listed in "Settings" => "Configure KDevelop..." => "Plugins", under the "Project Management" section.
You can also verify a successful installation by clicking "Project" => "Open / Import Project". Cargo files (`Cargo.toml`) should be selectable and available as a filter.

## Opening and Building a project

To use the plugin, import a Rust project by clicking "Project" => "Open / Import Project" and selecting a `Cargo.toml` file.
The Build and Clean commands already work.

## Running a binary

If your crate is a binary (as opposed to a library) crate, you can use `cargo run` through KDevelop as well.
Open the launch configuration dialog ("Run" => "Configure Launches..."), then click "Add New" => "Cargo Launcher".
If the crate has multiple executables, enter it in the launch configuration, and it well be passed to `cargo run` as the `--bin` argument.
Optionally, you can also specify arguments that will be passed to your executable.
