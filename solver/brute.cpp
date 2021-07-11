#include <set>

using std::max;
using std::set;


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
    u32 starting_edge;
    u32 score;
    u8 is_terminal;
    vector<point> vertices;
    vector<r32> edge_rotation;
} brute_search_state;


static brute_search_state
next_state(problem_t& problem, brute_search_state& state) {
    brute_search_state next_state = state;

    next_state.is_terminal = 1;
    return next_state;
}


static vector<point>
brute(problem_t& problem) {
    set<u32> valid_points = all_points_of_polygon(problem.hole);

    u32 soa_score = problem.soa.size() ? dislikes(problem, problem.soa) : -1;

    brute_search_state state = {};
    state.score = -1;
    state.vertices = problem.fig_vertices;

    while (state.score >= soa_score && !state.is_terminal) {
        state = next_state(problem, state);
    }

    if (state.score < soa_score) {
        return state.vertices;
    }
    return {};
}
