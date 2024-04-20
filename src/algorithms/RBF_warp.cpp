//-----------------------------------------------------------------------------

#include "RBF_warp.h"


#include <cfloat>

#include "Constants.h"
#include "utils/io/io_selection.h"

//-----------------------------------------------------------------------------

bool load_map_full_to_cutoff(RBF_data& out_data)
{
    return read_selection(RESOURCE_DATA_DIR + "/mapping_full_to_cut.sel", out_data.mapping_full_to_cutoff);
}

//-----------------------------------------------------------------------------

bool find_rbf_centers_prioritize_head(pmp::SurfaceMesh &skel_wrap,
                                      size_t num_additional_centers,
                                      RBF_data& out_data)
{
    using namespace pmp;

    std::vector<dvec3> &cts = out_data.centers;
    std::vector<int> &ids = out_data.indices;

    int N = skel_wrap.n_vertices();

    std::vector<double>  dist(N, FLT_MAX);
    std::vector<dvec3> centers(N);
    std::vector<int> indices(N);

    std::vector<int> inner_mouth_ids;
    if (!read_selection(RESOURCE_DATA_DIR + "/mouth.sel", inner_mouth_ids))
    {
        return false;
    }

    auto ignore = skel_wrap.vertex_property<bool>("v:rbf_ignore", false);
    for (int idx : inner_mouth_ids)
    {
        ignore[Vertex(idx)] = true;
    }

    std::vector<int> head_ids;
    if (!read_selection(RESOURCE_DATA_DIR + "/bo_head.sel", head_ids))
    {
        return false;
    }

    size_t n_head_vertices = 0;
    auto is_head = skel_wrap.vertex_property<bool>("v:rbf_is_head", false);
    for (int idx : head_ids)
    {
        Vertex v(idx);

        if (!ignore[v])
        {
            is_head[Vertex(idx)] = true;
            n_head_vertices++;
        }
    }

    // Half of the head vertices should be RBF centers
    size_t desired_head_centers = n_head_vertices / 2;

    for(auto v : skel_wrap.vertices())
    {
        centers[v.idx()] = dvec3(skel_wrap.position(v));
        indices[v.idx()] = v.idx();
    }

    // First handle head centers
    size_t k = 0;
    while(true)
    {
        // use next (k-th) center
        dvec3 p = centers[k];
        dist[k] = 0.0;
        if (++k == desired_head_centers) break;

        int imax = k;
        double dmax = 0.0;

        for (int i=k; i<N; ++i)
        {
            // NOTE(swenninger):
            //   This can be optimized by only looking over head_ids in every iteration

            Vertex v(indices[i]);
            if (!is_head[v] || ignore[v])
                continue;

            double d = sqrnorm(p - centers[i]);

            // update dist[i] as shortest dist to selected centers
            if (d < dist[i])
            {
                dist[i] = d;
            }

            // find farthest center
            if (dist[i] > dmax)
            {
                dmax = dist[i];
                imax = i;
            }
        }

        // move farthest center to array entry k
        std::swap(centers[k], centers[imax]);
        std::swap( indices[k],  indices[imax]);
        std::swap(    dist[k],     dist[imax]);
    }

    size_t num_centers = num_additional_centers + desired_head_centers;

    // Then find the rest of the body rbf centers
    while(true)
    {
        // use next (k-th) center
        dvec3 p = centers[k];
        dist[k] = 0.0;
        if (++k == num_centers) break;

        int imax = k;
        double dmax = 0.0;

        for (int i=k; i<N; ++i)
        {
            if (ignore[Vertex(indices[i])])
                continue;

            double d = sqrnorm(p - centers[i]);

            // update dist[i] as shortest dist to selected centers
            if (d < dist[i])
            {
                dist[i] = d;
            }

            // find farthest center
            if (dist[i] > dmax)
            {
                dmax = dist[i];
                imax = i;
            }
        }

        // move farthest center to array entry k
        std::swap(centers[k], centers[imax]);
        std::swap( indices[k],  indices[imax]);
        std::swap(    dist[k],     dist[imax]);
    }


    cts.resize(num_centers);
    ids.resize(num_centers);
    for(size_t i = 0; i < num_centers; i++)
    {
        ids[i] = indices[i];
        cts[i] = centers[i];
    }

    skel_wrap.remove_vertex_property(ignore);
    skel_wrap.remove_vertex_property(is_head);

    out_data.num_centers = num_centers;


    return true;
}

//-----------------------------------------------------------------------------

bool find_rbf_centers(pmp::SurfaceMesh &skel_wrap,
                      size_t num_centers,
                      RBF_data& out_data)
{
    using namespace pmp;

    std::vector<dvec3> &cts = out_data.centers;
    std::vector<int> &ids = out_data.indices;

    int N = skel_wrap.n_vertices();

    std::vector<double>  dist(N, FLT_MAX);
    std::vector<dvec3> centers(N);
    std::vector<int> indices(N);

    std::vector<int> inner_mouth_ids;
    if (!read_selection(RESOURCE_DATA_DIR + "/mouth.sel", inner_mouth_ids))
    {
        return false;
    }

    auto ignore = skel_wrap.vertex_property<bool>("v:rbf_ignore", false);
    for (int idx : inner_mouth_ids)
    {
        ignore[Vertex(idx)] = true;
    }

    for(auto v : skel_wrap.vertices())
    {
        centers[v.idx()] = dvec3(skel_wrap.position(v));
        indices[v.idx()] = v.idx();
    }

    size_t k = 0;
    while(true)
    {
        // use next (k-th) center
        dvec3 p = centers[k];
        dist[k] = 0.0;
        if (++k == num_centers) break;

        int imax = k;
        double dmax = 0.0;

        for (int i=k; i<N; ++i)
        {
            if (ignore[Vertex(indices[i])])
                continue;

            double d = sqrnorm(p - centers[i]);

            // update dist[i] as shortest dist to selected centers
            if (d < dist[i])
            {
                dist[i] = d;
            }

            // find farthest center
            if (dist[i] > dmax)
            {
                dmax = dist[i];
                imax = i;
            }
        }

        // move farthest center to array entry k
        std::swap(centers[k], centers[imax]);
        std::swap( indices[k],  indices[imax]);
        std::swap(    dist[k],     dist[imax]);
    }


    cts.resize(num_centers);
    ids.resize(num_centers);
    for(size_t i = 0; i < num_centers; i++)
    {
        ids[i] = indices[i];
        cts[i] = centers[i];
    }

    skel_wrap.remove_vertex_property(ignore);

    out_data.num_centers = num_centers;


    return true;
}

//-----------------------------------------------------------------------------

bool prefactorize_rbf_warp(std::vector<pmp::dvec3> &cts,
                           Eigen::PartialPivLU<Eigen::MatrixXd> &solver)
{
    size_t n = cts.size();
    if(n < 5)
    {
        std::cerr << "too few centers" << std::endl;
        return false;
    }

    // setup matrix
    Eigen::MatrixXd  A(n+4,n+4);
    size_t i,j;
    for (i=0; i<n; ++i)
    {
        for (j=0; j<n; ++j)
        {
            double r = distance(cts[i], cts[j]);
            A(i,j) = r*r*r;
        }
    }
    for (i=0; i<n; ++i)
    {
        j=n; A(i,j) = A(j,i) = 1;
        ++j; A(i,j) = A(j,i) = cts[i][0];
        ++j; A(i,j) = A(j,i) = cts[i][1];
        ++j; A(i,j) = A(j,i) = cts[i][2];
    }
    for (i=n; i<n+4; ++i)
    {
        for (j=n; j<n+4; ++j)
        {
            A(i,j) = 0.0;
        }
    }

    // decompose rbf-martrix
    solver.compute(A);

    return true;
}

//-----------------------------------------------------------------------------

bool init_rbf_warp(pmp::SurfaceMesh &skel_wrap,
                   size_t num_centers,
                   RBF_data& out_data)
{
    if (!load_map_full_to_cutoff(out_data)) return false;
    if (!find_rbf_centers(skel_wrap, num_centers, out_data)) return false;
    if (!prefactorize_rbf_warp(out_data.centers, out_data.solver)) return false;

    return true;
}

//-----------------------------------------------------------------------------

bool init_rbf_warp_prioritize_head(pmp::SurfaceMesh& skel_wrap,
                                   size_t num_additional_centers,
                                   RBF_data& out_data)
{
    if (!load_map_full_to_cutoff(out_data)) return false;
    if (!find_rbf_centers_prioritize_head(skel_wrap, num_additional_centers, out_data)) return false;
    if (!prefactorize_rbf_warp(out_data.centers, out_data.solver)) return false;

    return true;
}

//-----------------------------------------------------------------------------

bool apply_rbf_warp(pmp::SurfaceMesh& skel_wrap, pmp::SurfaceMesh& bones, RBF_data& rbf_data)
{
    using namespace pmp;

    std::vector<dvec3> &cts = rbf_data.centers;
    std::vector<int>   &ids = rbf_data.indices;
    Eigen::PartialPivLU<Eigen::MatrixXd> &sol = rbf_data.solver;

    size_t n = cts.size();
    if(n < 5)
    {
        std::cerr << "too few centers" << std::endl;
        return false;
    }

    // setup right hand side
    Eigen::MatrixXd  B(n+4,3);// B.setZero();
    size_t i,j;
    for (i=0; i<n; ++i)
    {
        int idx = ids[i];
        Vertex v(idx);

        dvec3 displ = (dvec3)skel_wrap.position(v);
        displ -= cts[i];

        for (j=0; j<3; ++j)
        {
            B(i,j) = displ[j];
        }
    }
    for (i=n; i<n+4; ++i)
    {
        for (j=0; j<3; ++j)
        {
            B(i,j) = 0.0;
        }
    }

    // solve system
    Eigen::MatrixXd X(3, n+4);

    #pragma omp parallel for
    for(int k = 0; k < 3; k++)
    {
        X.row(k) = (sol.solve(B.col(k))).transpose();
    }

    SurfaceMesh &apply_m = bones;

    // apply solution
    #pragma omp parallel for
    for(size_t vi = 0; vi < apply_m.n_vertices(); vi++)
    {
        Vertex v(vi);
        dvec3 p(apply_m.position(v));
        Eigen::Vector3d f; f.setZero();

        for(size_t i = 0; i < n; i++)
        {
            double r = distance(p, cts[i]);
            f += X.col(i).transpose()*r*r*r;
        }

        // linear polynomial
        f += X.col(n).transpose();
        f += X.col(n + 1).transpose()*p[0];
        f += X.col(n + 2).transpose()*p[1];
        f += X.col(n + 3).transpose()*p[2];

        apply_m.position(v) += f;
    }

    return true;
}

//-----------------------------------------------------------------------------
