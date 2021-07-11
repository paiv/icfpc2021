#include <stdint.h>
#include <unistd.h>
#include <vector>
#include <stdio.h>


using std::min;
using std::vector;

typedef uint8_t u8;
typedef int32_t i32;
typedef uint32_t u32;
typedef float r32;


typedef struct {
    i32 x, y;
} point;


typedef struct {
    vector<point> hole;
    vector<point> fig_vertices;
    vector<point> fig_edges;
    u32 epsilon;
    vector<point> soa;
} problem_t;


static void
read_line(vector<point>& vec) {
    point pt;
    while (read(STDIN_FILENO, &pt, sizeof(pt)) > 0) {
        if (pt.y == -0x80000000) {
            if (pt.x != -0x80000000) {
                vec.push_back(pt);
            }
            break;
        }
        vec.push_back(pt);
    }
}


template<typename T>
static void
read_value(T& value) {
    read(STDIN_FILENO, &value, sizeof(value));
}


static problem_t
read_problem() {
    problem_t problem = {};
    read_line(problem.hole);
    read_line(problem.fig_vertices);
    read_line(problem.fig_edges);
    read_line(problem.soa);
    if (problem.soa[0].y == -0x80000000) {
        problem.epsilon = problem.soa[0].x;
        problem.soa.clear();
    }
    else {
        read_value(problem.epsilon);
    }
    return problem;
}


static void
write_solution(vector<point>& solution) {
    point* soldata = solution.data();
    u32 solsize = solution.size() * sizeof(point);
    write(STDOUT_FILENO, soldata, solsize);
}


static u32
distancesq(point& pa, point& pb) {
    return (pa.x - pb.x) * (pa.x - pb.x) + (pa.y - pb.y) * (pa.y - pb.y);
}


static u8
polygon_has_point(vector<point>& polygon, point& pt) {
    i32 count = 0;
    for (size_t i = 0; i < polygon.size(); ++i) {
        auto& pa = polygon[i];
        auto& pb = polygon[(i+1) % polygon.size()];
        if (pt.y >= pa.y && pt.y <= pb.y) {
            i32 dt = (pt.y - pa.y) * (pb.x - pa.x) - (pt.x - pa.x) * (pb.y - pa.y);
            if (dt > 0) {
                if (pt.y == pa.y || pt.y == pb.y) {
                    count += 0.5;
                }
                else {
                    count += 1;
                }
            }
            else if (dt == 0 && ((pt.x >= pa.x && pt.x <= pb.x) || (pt.x <= pa.x && pt.x >= pb.x))) {
                return 1;
            }
        }
        else if (pt.y <= pa.y && pt.y >= pb.y) {
            i32 dt = (pt.y - pa.y) * (pb.x - pa.x) - (pt.x - pa.x) * (pb.y - pa.y);
            if (dt < 0) {
                if (pt.y == pa.y || pt.y == pb.y) {
                    count -= 0.5;
                }
                else {
                    count -= 1;
                }
            }
            else if (dt == 0 && ((pt.x >= pa.x && pt.x <= pb.x) || (pt.x <= pa.x && pt.x >= pb.x))) {
                return 1;
            }
        }
    }
    return (count != 0);
}


static u8
line_segments_intersect(point& a, point& b, point& c, point& d) {
    i32 detA = (d.x - c.x) * (b.y - a.y) - (b.x - a.x) * (d.y - c.y);
    if (detA == 0) { return 0; }
    i32 detS = (d.x - c.x) * (c.y - a.y) - (c.x - a.x) * (d.y - c.y);
    i32 detT = (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y);
    r32 s = (r32)detS / detA;
    r32 t = (r32)detT / detA;
    return (s > 0 && s < 1 && t > 0 && t < 1);
}


static u8
segment_breaks_hole(problem_t& problem, point& pa, point& pb) {
    auto& hole = problem.hole;
    for (size_t i = 0; i < hole.size(); ++i) {
        auto& ha = hole[i];
        auto& hb = hole[(i+1) % hole.size()];
        if (line_segments_intersect(ha, hb, pa, pb)) {
            return 1;
        }
    }
    return 0;
}


static u8
valid_solution(problem_t& problem, vector<point>& solution) {
    auto& edges = problem.fig_edges;
    for (size_t i = 0; i < edges.size(); ++i) {
        auto& pa = solution[edges[i].x];
        auto& pb = solution[edges[i].y];
        if (segment_breaks_hole(problem, pa, pb)) {
            return 0;
        }
    }
    return 1;
}


static u32
dislikes(problem_t& problem, vector<point>& solution) {
    if (!valid_solution(problem, solution)) {
        return -1;
    }
    u32 score = 0;
    for (auto& pa : problem.hole) {
        u32 mindist = -1;
        for (auto& pb : solution) {
            mindist = min(mindist, distancesq(pa, pb));
        }
        score += mindist;
    }
    return score;
}


static vector<point>
solver0(problem_t& problem) {
    return problem.fig_vertices;
}


static vector<point>
solver1(problem_t& problem) {
    u32 soa_score = problem.soa.size() ? dislikes(problem, problem.soa) : -1;
    fprintf(stderr, "soa score: %u\n", soa_score);
    return {};
}


int
main(int argc, char const *argv[]) {
    problem_t problem = read_problem();
    auto solution = solver1(problem);
    write_solution(solution);

    return 0;
}
