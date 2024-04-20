#ifndef TAILORME_VIEWER_BASEMESH_H
#define TAILORME_VIEWER_BASEMESH_H

#include <pmp/bounding_box.h>
#include <pmp/surface_mesh.h>
#include <pmp/visualization/renderer.h>

// use eigen namespace and declare alias types
#include "GlobTypes.h"

// define all possible mesh types
enum MeshType {
    MESH_UNDEFINED,
    MESH_MALE,
    MESH_FEMALE,
};

enum SubMeshType {
    SUBMESH_SKEL,
    SUBMESH_SKELWRAP,
    SUBMESH_SKIN,
};

enum MeshLayer {
    LayerBone,
    LayerSkel,
    LayerSkin,
};

class BaseMesh {
protected:
    // alpha for skin
    float _alpha_bone = 1.0F;
    float _alpha_skel = 1.0F;
    float _alpha_skin = 1.0F;
    // point data
    VectorXf mesh_points_ {};
    // error for vertices (skel, skin)
    VectorXf vertex_rmse_ {};

    //! helper
    static void error_to_texture(pmp::SurfaceMesh& renderer, VectorXf& errors);

public:
    // destructor can be overwritten
    virtual ~BaseMesh() = default;

    // render mesh
    virtual void draw(const pmp::mat4 &projection_matrix,
              const pmp::mat4 &modelview_matrix,
              const std::string &draw_mode) = 0;

    // update mesh buffers for opengl
    virtual void update_meshes() = 0;

    // update mesh point data
    virtual void update_mesh_points(VectorXf &point_data) = 0;

    // update only points for one mesh
    virtual auto update_layer_points(ArrayXf &point_data, MeshLayer layer) -> void;

    // set alpha
    void set_alpha(float bone, float skel, float skin) {
        _alpha_bone = bone;
        _alpha_skel = skel;
        _alpha_skin = skin;
    }

    // get vertex count
    virtual auto submesh_vertex_count(SubMeshType submesh_type) -> long;

    // save meshes
    void save_meshes(const std::string& bone_filename, const std::string& skel_filename, const std::string& skin_filename);

    //! get base mesh objects
    virtual auto get_bone() -> pmp::SurfaceMesh*;
    virtual auto get_skel() -> pmp::SurfaceMesh*;
    virtual auto get_skin() -> pmp::SurfaceMesh*;

    //! set mesh parts
    virtual auto set_skel(pmp::SurfaceMesh& mesh) -> void;
    virtual auto set_skin(pmp::SurfaceMesh& mesh) -> void;

    //! run optimizers
    virtual auto optimize_meshes() -> void;
};


#endif //TAILORME_VIEWER_BASEMESH_H
