# KDevelop plugin with support for Cargo

[![Codefresh build status]( https://g.codefresh.io/api/badges/build?repoOwner=Noughmad&repoName=kdevcargo&branch=master&accountName=Noughmad&pipelineName=kdevcargo&type=cf-1)]( https://g.codefresh.io/repositories/Noughmad/kdevcargo/builds?filter=trigger:build;branch:master;service:591d7e9809f4a600015ecd49~kdevcargo)

This plugin enables the use of Cargo to manage, build and run Rust packages (crates) with Cargo.

## Features

- Recognize `Cargo.toml` files as project files and allows to open them as projects
- Builds projects using `cargo build`
- Configure launches using `cargo run` with possibility to set a binary name and arguments
- Colored and clickable `cargo build` output for quick jumping to lines with errors or warnings

## Installation instructions

This plugin supports KDevelop 5, so make sure have kdevplatform libraries development headers (`kdevplatform-dev` on ubuntu) version 5 or later.
You also need `cmake` and `extra-cmake-modules` to build it.
To use the plugin, the `cargo` executable must be installed and in your `PATH`.

Download this repository, then run

    mkdir build && cmake .. && make
    sudo make install

This installs to a system directory, most likely `/usr/local`, but it will not be picked up by KDevelop.
To make KDevelop load this plugin, set the `QT_PLUGIN_PATH` environment variable to point to its install location.
If it was installed to `/usr/local/lib64/plugins/kdevplatform/<version>/kdevcargo.so`, run from a console

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
