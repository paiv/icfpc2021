#include <stdint.h>
#include <unistd.h>
#include <vector>


using std::min;
using std::vector;

typedef uint8_t u8;
typedef int32_t i32;
typedef uint32_t u32;
typedef uint64_t u64;
typedef double r64;


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

static u64
dislikes(problem_t& problem, vector<point>& solution) {
    u64 score = 0;
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
    return {};
}


int
main(int argc, char const *argv[]) {
    problem_t problem = read_problem();
    auto solution = solver0(problem);
    write_solution(solution);

    return 0;
}
