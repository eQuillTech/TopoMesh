# TopoMesh
A C++ library that generates triangulations from 2-D topographic images. 

Grayscale TIFF images can converted to `.obj` with the `TriCalc` tool.

`Mesh` stores all vertex information, while its `Submesh` members store the triangulations. Meshes can be loaded from `.obj` files, without the texture information.
