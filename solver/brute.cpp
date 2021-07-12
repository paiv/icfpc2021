#include <cmath>
#include <cstdlib>
#include <set>
#include <queue>

using std::abs;
using std::max;
using std::queue;
using std::set;
using std::sqrt;


static u32
pack_point(point& pt) {
    return (pt.y + 1000) * 0x10000 + (pt.x + 1000);
}


static point
unpack_point(u32 v) {
    i32 x = i32(v % 0x10000) - 1000;
    i32 y = i32(v / 0x10000);
    return {x, y};
}


static set<u32>
all_points_of_polygon(vector<point>& polygon) {
    i32 minx=0x800000, miny=0x800000;
    i32 maxx=-0x800000, maxy=-0x800000;
    for (auto& pt : polygon) {
        minx = min(minx, pt.x);
        maxx = max(maxx, pt.x);
        miny = min(miny, pt.y);
        maxy = max(maxy, pt.y);
    }
    set<u32> result;
    for (i32 y = miny; y <= maxy; ++y) {
        for (i32 x = minx; x <= maxx; ++x) {
            point pt = {x, y};
            if (polygon_has_point(polygon, pt)) {
                result.insert(pack_point(pt));
            }
        }
    }
    return result;
}


typedef struct {
    u32 score;
    u8 is_terminal;
    vector<point> vertices;
    u32 root_vertex;
    vector<u32> vertex_state;
    vector<u32> point_stack;
} brute_search_state;


typedef struct {
    set<u32> valid_points;
    // map<u32,vector<u32>> valid_point_positions;
} brute_cache;


static u32
get_score(problem_t& problem, vector<point>& solution) {
    return u32(-1) - dislikes(problem, solution);
}


static u32
next_vertex_in_graph(vector<point>& edges, vector<u32>& path) {
    queue<u32> fringe;
    fringe.push(path.front());
    set<u32> visited;
    set<u32> goal(begin(path), end(path));

    while (!fringe.empty()) {
        u32 vx = fringe.front();
        fringe.pop();

        if (goal.find(vx) == goal.end()) {
            return vx;
        }

        if (visited.find(vx) != visited.end()) { continue; }
        visited.insert(vx);

        for (auto& edge : edges) {
            if (edge.a == vx) {
                fringe.push(edge.b);
            }
            else if (edge.b == vx) {
                fringe.push(edge.a);
            }
        }
    }
    return -1;
}


static void
init_state(problem_t& problem, brute_search_state& state) {
    state.vertices = problem.fig_vertices;
    state.vertex_state.resize(state.vertices.size());
}


static const u32 NOSTATE = 0x80000000;


static u8
next_valid_root(problem_t& problem, brute_search_state& state, brute_cache& cache, u32* vi, u32* ci, point* pt) {
    fprintf(stderr, "next_valid_root %u %u\n", *vi, *ci);
    u32 q = *ci + 1;
    if (q < problem.hole.size()) {
        *ci = q;
        *pt = problem.hole[q];
        fprintf(stderr, "  (1) at %d,%d\n", problem.hole[q].x, problem.hole[q].y);
        return 1;
    }
    u32 p = *vi + 1;
    if (p < problem.fig_vertices.size()) {
        q = 0;
        *vi = p;
        *ci = q;
        *pt = problem.hole[q];
        fprintf(stderr, "  (2) at %d,%d\n", problem.hole[q].x, problem.hole[q].y);
        return 1;
    }
    fprintf(stderr, "not found\n");
    return 0;
}


static u32
visited_connected_vertex(problem_t& problem, brute_search_state& state, u32 vi) {
    u32 closest = -1;
    u32 best_distance = -1;

    for (auto& edge : problem.fig_edges) {
        if (edge.a != vi && edge.b != vi) { continue; }
        u32 xo = edge.a;
        if (xo == vi) {
            xo = edge.b;
        }
        u32 dist = distancesq(problem.fig_vertices[xo], problem.fig_vertices[vi]);
        if (dist >= best_distance) { continue; }
        for (u32 p : state.point_stack) {
            if (xo == p) {
                closest = xo;
                best_distance = dist;
            }
        }
    }
    return closest;
}


static void
compression_range(point& pa, point& pb, u32 epsilon, r32* rmin, r32* rmax) {
    r64 r = sqrt(distancesq(pa, pb));
    r64 q = r64(1000000 - epsilon) * r / 1000000;
    *rmin = q;
    *rmax = 2 * r - q;
}


static u8
is_valid_placement(problem_t& problem, brute_search_state& state, brute_cache& cache, u32 vi, point& pt) {
    for (auto& edge : problem.fig_edges) {
        if (edge.a != vi && edge.b != vi) { continue; }
        u32 xo = edge.a;
        if (xo == vi) {
            xo = edge.b;
        }
        for (u32 p : state.point_stack) {
            if (xo == p) {
                u32 odist = distancesq(problem.fig_vertices[xo], problem.fig_vertices[vi]);
                u32 rdist = distancesq(state.vertices[xo], pt);
                if (abs((r64)rdist / odist - 1) * 1000000 > problem.epsilon) {
                    return 0;
                }
                if (segment_breaks_hole(problem, state.vertices[xo], pt)) {
                    return 0;
                }
                fprintf(stderr, "  valid [%d,%d] - [%d,%d]\n", pt.x, pt.y, state.vertices[xo].x, state.vertices[xo].y);
            }
        }
    }

    return 1;
}


static u8
next_valid_placement(problem_t& problem, brute_search_state& state, brute_cache& cache, u32* ovs, point* opt) {
    u32 vi = state.point_stack.back();
    u32 xo = visited_connected_vertex(problem, state, vi);

    point pa = problem.fig_vertices[xo];
    point pb = problem.fig_vertices[vi];
    r32 rmin, rmax;
    compression_range(pa, pb, problem.epsilon, &rmin, &rmax);

    u32 yomin = ceil(pa.y - rmax);
    u32 yomax = floor(pa.y + rmax);
    u32 yimin = floor(pa.y - rmin);
    u32 yimax = ceil(pa.y + rmin);
    u32 xomin = ceil(pa.x - rmax);
    u32 xomax = floor(pa.x + rmax);
    u32 ximin = floor(pa.x - rmin);
    u32 ximax = ceil(pa.x + rmin);

    u32 yadj = *ovs / (yomax - yomin + 1);

    u32 count = 0;
    for (u32 y = yomin + yadj; y <= yomax; ++y) {
        for (u32 x = xomin; x <= xomax; ++x, ++count) {
            if (count <= *ovs) { continue; }
            if (y > yimin && y < yimax) {
                if (x > ximin && x < ximax) { continue; }
            }
            point pt = {.a=x, .b=y};
            u32 pp = pack_point(pt);
            if (cache.valid_points.find(pp) != cache.valid_points.end()) {
                if (is_valid_placement(problem, state, cache, vi, pt)) {
                    fprintf(stderr, "placement [%d,%d]\n", pt.x, pt.y);
                    *ovs = count;
                    *opt = pt;
                    return 1;
                }
            }
        }
    }
    return 0;
}

static brute_search_state
next_state(problem_t& problem, brute_search_state& state, brute_cache& cache) {
    brute_search_state next_state = state;

    if (state.point_stack.size() == 0) {
        u32 vi = 0;
        u32 ci = 0;
        next_state.root_vertex = vi;
        next_state.vertices[vi] = problem.hole[ci];
        next_state.vertex_state[vi] = ci;
        next_state.point_stack.push_back(vi);
    }
    else if (next_state.vertex_state[state.point_stack.back()] == NOSTATE || state.point_stack.size() == state.vertices.size()) {
        while (next_state.point_stack.size() > 0) {
            u32 vi = next_state.point_stack.back();
            u32 vs = next_state.vertex_state[vi];
            point pt;
            if (next_state.point_stack.size() == 1) {
                if (next_valid_root(problem, state, cache, &vi, &vs, &pt)) {
                    next_state.point_stack.back() = vi;
                    next_state.root_vertex = vi;
                    next_state.vertex_state[vi] = vs;
                    next_state.vertices[vi] = pt;
                    break;
                }
            }
            else if (next_valid_placement(problem, state, cache, &vs, &pt)) {
                next_state.vertex_state[vi] = vs;
                next_state.vertices[vi] = pt;
                break;
            }
            next_state.point_stack.pop_back();
        }
    }
    else {
        u32 vi = next_vertex_in_graph(problem.fig_edges, state.point_stack);
        next_state.point_stack.push_back(vi);
        next_state.vertex_state[vi] = NOSTATE;
    }

    if (next_state.point_stack.size() == 0) {
        fprintf(stderr, "terminal\n");
        next_state.is_terminal = 1;
        return next_state;
    }

    // if (state.point_stack.size() < state.vertices.size()) {
    //     return next_state;
    // }
    for (u32 s : state.vertex_state) {
        if (s == NOSTATE) {
            return next_state;
        }
    }

    next_state.score = get_score(problem, next_state.vertices);
    fprintf(stderr, "got new score %u\n", next_state.score);

    return next_state;
}


static vector<point>
brute(problem_t& problem) {
    u32 soa_score = get_score(problem, problem.soa);

    brute_cache cache = {};
    cache.valid_points = all_points_of_polygon(problem.hole);

    brute_search_state state = {};
    init_state(problem, state);

    while (state.score <= soa_score && !state.is_terminal) {
        #if 1
        fprintf(stderr, "state score %u (soa %u)\n", state.score, soa_score);
        fprintf(stderr, "root %u, stack size %lu [", state.root_vertex, state.point_stack.size());
        for (u32 p : state.point_stack) {
            fprintf(stderr, "%u,", p);
        }
        fprintf(stderr, "]\n");
        fprintf(stderr, "vertices: [");
        for (auto& p : state.vertices) {
            fprintf(stderr, "[%d,%d],", p.x, p.y);
        }
        fprintf(stderr, "]\n");
        #endif

        state = next_state(problem, state, cache);
    }

    if (state.score > soa_score) {
        fprintf(stderr, "found solution\n");
        return state.vertices;
    }
    fprintf(stderr, "found nothing\n");
    return {};
}
