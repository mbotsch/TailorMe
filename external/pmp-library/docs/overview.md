# Overview {#overview}

This section provides a high-level overview of PMP. We describe its design as well as the capabilities provided by the library. PMP is organized into different modules. At the core of the library is the @ref core module providing a data structure for polygonal surface meshes. On top of the @ref core module the @ref algorithms module provides implementations of canonical geometry processing algorithms such as remeshing, decimation, subdivision, and smoothing. The optional @ref visualization module provides OpenGL&reg;-based viewers and visualization tools.

## Core

The @ref core of the library provides a simple and efficient data structure for representing polygon surface meshes, pmp::SurfaceMesh. It also defines basic types such as 3D points and vectors as well as a basic utility classes for timing and memory profiling. See the @ref tutorial for how to use the core pmp::SurfaceMesh data structure.

You can find more details on the design and implementation of the core surface mesh data structure in our original paper \cite sieger_2011_design .

## Algorithms

The @ref algorithms module provides implementations of canonical geometry processing algorithms such as remeshing or mesh decimation. For each type of algorithms, we provide simple functions that take a pmp::SurfaceMesh as an argument, eventually followed by optional parameters to control algorithm behavior.

## Visualization

In order to easily create visualizations the library contains an optional @ref visualization module including basic viewers, e.g., pmp::MeshViewer. The corresponding OpenGL&reg; code for rendering the data is contained in pmp::Renderer class.
