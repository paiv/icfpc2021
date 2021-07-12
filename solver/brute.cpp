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


static const u32 NOSTATE = 0x80000000;


static void
init_state(problem_t& problem, brute_search_state& state) {
    state.vertices = problem.fig_vertices;
    state.vertex_state.resize(state.vertices.size(), NOSTATE);
}


static u8
next_valid_root(problem_t& problem, brute_search_state& state, brute_cache& cache, u32* vi, u32* ci, point* pt) {
    debug_print("next_valid_root %u %u\n", *vi, *ci);
    u32 q = *ci + 1;
    if (q < problem.hole.size()) {
        *ci = q;
        *pt = problem.hole[q];
        debug_print("  (1) at %d,%d\n", problem.hole[q].x, problem.hole[q].y);
        return 1;
    }
    u32 p = *vi + 1;
    if (p < problem.fig_vertices.size()) {
        q = 0;
        *vi = p;
        *ci = q;
        *pt = problem.hole[q];
        debug_print("  (2) at %d,%d\n", problem.hole[q].x, problem.hole[q].y);
        return 1;
    }
    debug_print("not found\n");
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
    debug_print("visited_connected_vertex for %u found %u\n", vi, closest);
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
    // debug_print("is_valid_placement for %u at %d,%d\n", vi, pt.x, pt.y);
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
                    // debug_print("    nope. (%u / %u - 1) x 1M > epsilon %u\n", rdist, odist, problem.epsilon);
                    return 0;
                }
                if (segment_breaks_hole(problem, state.vertices[xo], pt)) {
                    // debug_print("    nope. breaks edges\n");
                    return 0;
                }
                debug_print("  valid %u [%d,%d] - %u [%d,%d]\n", vi, pt.x, pt.y, xo, state.vertices[xo].x, state.vertices[xo].y);
            }
        }
    }

    return 1;
}


static u8
next_valid_placement(problem_t& problem, brute_search_state& state, brute_cache& cache, u32* ovs, point* opt) {
    debug_print("next_valid_placement\n");
    u32 vi = state.point_stack.back();
    u32 xo = visited_connected_vertex(problem, state, vi);

    point pa = problem.fig_vertices[xo];
    point pb = problem.fig_vertices[vi];
    r32 rmin, rmax;
    compression_range(pa, pb, problem.epsilon, &rmin, &rmax);
    debug_print("  compression range: %.4f min, %.4f max\n", rmin, rmax);
    rmin /= sqrt(2);

    pa = state.vertices[xo];
    i32 yomin = ceil(pa.y - rmax);
    i32 yomax = floor(pa.y + rmax);
    i32 yimin = floor(pa.y - rmin);
    i32 yimax = ceil(pa.y + rmin);
    i32 xomin = ceil(pa.x - rmax);
    i32 xomax = floor(pa.x + rmax);
    i32 ximin = floor(pa.x - rmin);
    i32 ximax = ceil(pa.x + rmin);
    debug_print("  outer box: [x:%d, y:%d, w:%d, h:%d]\n", xomin, yomin, xomax-xomin+1, yomax-yomin+1);
    debug_print("  inner box: [x:%d, y:%d, w:%d, h:%d]\n", ximin, yimin, ximax-ximin+1, yimax-yimin+1);

    u32 yadj = 0;
    if (*ovs != NOSTATE) {
        // yadj = *ovs / (yomax - yomin + 1);
    }

    u32 count = 0;
    for (i32 y = yomin + yadj; y <= yomax; ++y) {
        for (i32 x = xomin; x <= xomax; ++x, ++count) {
            if (count <= *ovs && *ovs != NOSTATE) { continue; }
            if (y > yimin && y < yimax) {
                if (x > ximin && x < ximax) { continue; }
            }
            point pt = {x, y};
            u32 pp = pack_point(pt);
            if (cache.valid_points.find(pp) != cache.valid_points.end()) {
                if (is_valid_placement(problem, state, cache, vi, pt)) {
                    debug_print("placement [%d,%d]\n", pt.x, pt.y);
                    *ovs = count;
                    *opt = pt;
                    return 1;
                }
            }
        }
    }
    debug_print("    none\n");
    return 0;
}

static brute_search_state
next_state(problem_t& problem, brute_search_state& state, brute_cache& cache) {
    brute_search_state next_state = state;
    next_state.score = 0;

    if (state.point_stack.size() == 0) {
        u32 vi = 0;
        u32 ci = 0;
        next_state.root_vertex = vi;
        next_state.vertices[vi] = problem.hole[ci];
        next_state.vertex_state[vi] = ci;
        next_state.point_stack.push_back(vi);
        return next_state;
    }
    else if (state.vertex_state[state.point_stack.back()] == NOSTATE || state.point_stack.size() == state.vertices.size()) {
        while (next_state.point_stack.size() > 0) {
            u32 vi = next_state.point_stack.back();
            u32 vs = next_state.vertex_state[vi];
            point pt;
            if (next_state.point_stack.size() == 1) {
                if (next_valid_root(problem, next_state, cache, &vi, &vs, &pt)) {
                    next_state.point_stack.back() = vi;
                    next_state.root_vertex = vi;
                    next_state.vertex_state[vi] = vs;
                    next_state.vertices[vi] = pt;
                    break;
                }
            }
            else if (next_valid_placement(problem, next_state, cache, &vs, &pt)) {
                next_state.vertex_state[vi] = vs;
                next_state.vertices[vi] = pt;
                break;
            }
            debug_print("pop point_stack\n");
            next_state.vertex_state[vi] = NOSTATE;
            next_state.point_stack.pop_back();
        }
    }
    else {
        u32 vi = next_vertex_in_graph(problem.fig_edges, state.point_stack);
        next_state.point_stack.push_back(vi);
        next_state.vertex_state[vi] = NOSTATE;
        return next_state;
    }

    if (next_state.point_stack.size() == 0) {
        debug_print("terminal\n");
        next_state.is_terminal = 1;
        return next_state;
    }

    if (next_state.point_stack.size() == next_state.vertices.size()) {
        next_state.score = get_score(problem, next_state.vertices);
        debug_print("got new score %u\n", next_state.score);
    }

    return next_state;
}


static vector<point>
brute(problem_t& problem) {
    u32 soa_score = get_score(problem, problem.soa);
    if (soa_score == u32_max) {
        return {};
    }

    brute_cache cache = {};
    cache.valid_points = all_points_of_polygon(problem.hole);

    brute_search_state state = {};
    init_state(problem, state);

    while (state.score <= soa_score && !state.is_terminal) {
        #if 1
        debug_print("state score %u (soa %u)\n", state.score, soa_score);
        debug_print("root %u, stack size %lu [", state.root_vertex, state.point_stack.size());
        for (u32 p : state.point_stack) {
            debug_print("%u,", p);
        }
        debug_print("]\n");
        debug_print("vertices: [");
        for (size_t i = 0; i < state.vertices.size(); ++i) {
            if (state.vertex_state[i] == NOSTATE) {
                debug_print("(na),");
            }
            else {
                auto& p = state.vertices[i];
                debug_print("[%d,%d],", p.x, p.y);
            }
        }
        debug_print("]\n");
        #endif

        state = next_state(problem, state, cache);
    }

    if (state.score > soa_score) {
        debug_print("found solution\n");
        return state.vertices;
    }
    debug_print("found nothing\n");
    return {};
}
