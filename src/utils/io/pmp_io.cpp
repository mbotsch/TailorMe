//======================================================================================================================
// Copyright (c) Fabian Kemper 2024
//
// This work is licensed under a
// Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//
// You should have received a copy of the license along with this
// work. If not, see <http://creativecommons.org/licenses/by-nc-sa/4.0/>.
//
//======================================================================================================================

#include "pmp_io.h"

#include <iostream>
#include <string>

#include <pmp/io/write_off.h>

// =====================================================================================================================

using namespace pmp;

// =====================================================================================================================

void read_obj_buffer(SurfaceMesh& mesh, const std::stringstream& buffer)
{
    // reset all
    mesh.clear();

    // convert to istream (for reading)
    std::istringstream iss (buffer.str());
    std::istream& input(iss);

    float x, y, z;
    std::vector<Vertex> vertices;
    std::vector<TexCoord> all_tex_coords; //individual texture coordinates
    std::vector<int>
        halfedge_tex_idx; //texture coordinates sorted for halfedges
    HalfedgeProperty<TexCoord> tex_coords =
        mesh.halfedge_property<TexCoord>("h:tex");
    bool with_tex_coord = false;

    // parse line by line (currently only supports vertex positions & faces
    std::string line;

    //while (buffer && !buffer.eof() && fgets(read_buffer.data(), 200, buffer))
    while(std::getline(input, line))
    {
        // comment
        if (line[0] == '#' || isspace(line[0]))
            continue;

        // vertex
        else if (strncmp(line.data(), "v ", 2) == 0)
        {
            if (sscanf(line.data(), "v %f %f %f", &x, &y, &z))
            {
                mesh.add_vertex(Point(x, y, z));
            }
        }

        // normal
        else if (strncmp(line.data(), "vn ", 3) == 0)
        {
            if (sscanf(line.data(), "vn %f %f %f", &x, &y, &z))
            {
                // problematic as it can be either a vertex property when interpolated
                // or a halfedge property for hard edges
            }
        }

        // texture coordinate
        else if (strncmp(line.data(), "vt ", 3) == 0)
        {
            if (sscanf(line.data(), "vt %f %f", &x, &y))
            {
                all_tex_coords.emplace_back(x, y);
            }
        }

        // face
        else if (strncmp(line.data(), "f ", 2) == 0)
        {
            int component(0);
            bool end_of_vertex(false);
            char *p0, *p1(line.data() + 1);

            vertices.clear();
            halfedge_tex_idx.clear();

            // skip white-spaces
            while (*p1 == ' ')
                ++p1;

            while (p1)
            {
                p0 = p1;

                // overwrite next separator

                // skip '/', '\n', ' ', '\0', '\r' <-- don't forget Windows
                while (*p1 != '/' && *p1 != '\r' && *p1 != '\n' && *p1 != ' ' &&
                       *p1 != '\0')
                    ++p1;

                // detect end of vertex
                if (*p1 != '/')
                {
                    end_of_vertex = true;
                }

                // replace separator by '\0'
                if (*p1 != '\0')
                {
                    *p1 = '\0';
                    p1++; // point to next token
                }

                // detect end of line and break
                if (*p1 == '\0' || *p1 == '\n')
                {
                    p1 = nullptr;
                }

                // read next vertex component
                if (*p0 != '\0')
                {
                    switch (component)
                    {
                    case 0: // vertex
                    {
                        int idx = atoi(p0);
                        if (idx < 0)
                            idx = mesh.n_vertices() + idx + 1;
                        vertices.emplace_back(idx - 1);
                        break;
                    }
                    case 1: // texture coord
                    {
                        int idx = atoi(p0) - 1;
                        halfedge_tex_idx.push_back(idx);
                        with_tex_coord = true;
                        break;
                    }
                    case 2: // normal
                        break;
                    }
                }

                ++component;

                if (end_of_vertex)
                {
                    component = 0;
                    end_of_vertex = false;
                }
            }

            Face f = mesh.add_face(vertices);

            // add texture coordinates
            if (with_tex_coord && f.is_valid())
            {
                auto h_fit = mesh.halfedges(f);
                auto h_end = h_fit;
                unsigned v_idx = 0;
                do
                {
                    tex_coords[*h_fit] =
                        all_tex_coords.at(halfedge_tex_idx.at(v_idx));
                    ++v_idx;
                    ++h_fit;
                } while (h_fit != h_end);
            }
        }
    }

    // if there are no textures, delete texture property!
    if (!with_tex_coord)
    {
        mesh.remove_halfedge_property(tex_coords);
    }

}
