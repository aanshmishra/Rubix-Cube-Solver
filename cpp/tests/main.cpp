// Native test harness for the C++ solver. Built with plain g++ (no emscripten,
// no cmake) so the algorithm can be proven before anything is shipped to WASM.
//
//   g++ -std=c++17 -O2 -I../src -o solver_tests main.cpp <sources...>
//
// Covers, in order of how much they would hurt if wrong:
//   1. move engine identities  (X^4 == id, sexy move ^6 == id, inverses)
//   2. facelet round-trip      (encode . decode == id)
//   3. cross-check vs TS       (fixture states captured from the TS engine)
//   4. full solves             (every solution actually solves the cube)

#include "core/cube_state.h"
#include "core/facelet_converter.h"
#include "core/validation.h"
#include "solver/solve_api.h"

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

using namespace rubiks;

static int g_failures = 0;
static int g_checks = 0;

static void check(bool ok, const std::string& what) {
    ++g_checks;
    if (!ok) {
        ++g_failures;
        std::printf("  FAIL: %s\n", what.c_str());
    }
}

// Solved cube with U=W R=R F=G D=Y L=O B=B, matching solvedFacelets() in
// src/lib/cubeEngine.ts (physical orientation "YG").
static const std::string TARGET_YG =
    "WWWWWWWWW"
    "RRRRRRRRR"
    "GGGGGGGGG"
    "YYYYYYYYY"
    "OOOOOOOOO"
    "BBBBBBBBB";

// Deterministic PRNG (mulberry32) so any failure is reproducible from its seed.
struct Mulberry32 {
    uint32_t s;
    explicit Mulberry32(uint32_t seed) : s(seed) {}
    double next() {
        uint32_t t = (s += 0x6d2b79f5u);
        t = (t ^ (t >> 15)) * (t | 1u);
        t ^= t + (t ^ (t >> 7)) * (t | 61u);
        return static_cast<double>(t ^ (t >> 14)) / 4294967296.0;
    }
};

static std::string randomScramble(Mulberry32& rng, int length) {
    static const char* faces[6] = {"U", "R", "F", "D", "L", "B"};
    static const char* sfx[3] = {"", "'", "2"};
    std::string out;
    int last = -1;
    for (int i = 0; i < length; ++i) {
        int f;
        do { f = static_cast<int>(rng.next() * 6); if (f > 5) f = 5; } while (f == last);
        last = f;
        int s = static_cast<int>(rng.next() * 3);
        if (s > 2) s = 2;
        if (!out.empty()) out += " ";
        out += faces[f];
        out += sfx[s];
    }
    return out;
}

// ---------------------------------------------------------------------------

static void testMoveEngine() {
    std::printf("move engine\n");

    // Every quarter turn has order 4.
    for (int f = 0; f < 6; ++f) {
        CubeState s = CubeState::solved();
        Move m = static_cast<Move>(f * 3);
        for (int i = 0; i < 4; ++i) s.applyMove(m);
        check(s == CubeState::solved(), moveToString(m) + "^4 == identity");

        // A half turn is two quarters, and its own inverse.
        CubeState a = CubeState::solved();
        a.applyMove(m); a.applyMove(m);
        CubeState b = CubeState::solved();
        b.applyMove(static_cast<Move>(f * 3 + 1));
        check(a == b, moveToString(m) + " twice == " + moveToString(static_cast<Move>(f * 3 + 1)));

        // Quarter then inverse-quarter is a no-op.
        CubeState c = CubeState::solved();
        c.applyMove(m);
        c.applyMove(inverseMove(m));
        check(c == CubeState::solved(), moveToString(m) + " then its inverse == identity");
    }

    // (R U R' U')^6 is the classic order-6 identity.
    CubeState sexy = CubeState::solved();
    for (int i = 0; i < 6; ++i) sexy.applyNotation("R U R' U'");
    check(sexy == CubeState::solved(), "(R U R' U')^6 == identity");

    // Superflip: all 12 edges in place but flipped, corners untouched.
    CubeState sf = CubeState::solved();
    sf.applyNotation("U R2 F B R B2 R U2 L B2 R U' D' R2 F R' L B2 U2 F2");
    bool flipped = true;
    for (uint8_t i = 0; i < NUM_EDGES; ++i) {
        if (sf.edge_perm[i] != i || sf.edge_orient[i] != 1) flipped = false;
    }
    for (uint8_t i = 0; i < NUM_CORNERS; ++i) {
        if (sf.corner_perm[i] != i || sf.corner_orient[i] != 0) flipped = false;
    }
    check(flipped, "superflip leaves every edge home and flipped, corners clean");

    // A random scramble followed by its inverse returns to solved.
    Mulberry32 rng(12345);
    for (int trial = 0; trial < 200; ++trial) {
        std::string scr = randomScramble(rng, 15);
        std::vector<Move> mv = parseNotation(scr);
        CubeState s = CubeState::solved();
        s.applyMoves(mv);
        for (auto it = mv.rbegin(); it != mv.rend(); ++it) s.applyMove(inverseMove(*it));
        if (s != CubeState::solved()) {
            check(false, "scramble+inverse == identity for: " + scr);
            break;
        }
    }
    check(true, "200 random scramble/inverse round-trips");
}

static void testFaceletRoundTrip() {
    std::printf("facelet conversion\n");

    CubieEncoder enc(TARGET_YG);
    check(enc.valid(), "target colour scheme accepted");

    check(enc.decode(CubeState::solved()) == TARGET_YG, "solved state decodes to the target");

    Mulberry32 rng(999);
    bool all_ok = true;
    for (int trial = 0; trial < 200; ++trial) {
        std::string scr = randomScramble(rng, 20);
        CubeState s = CubeState::solved();
        s.applyNotation(scr);

        std::string fl = enc.decode(s);
        CubeState back;
        if (!enc.encode(fl, back) || !(back == s)) {
            check(false, "round-trip failed for: " + scr);
            all_ok = false;
            break;
        }
        // Nine of each colour, always.
        int counts[128] = {0};
        for (char c : fl) counts[static_cast<unsigned char>(c)]++;
        for (const char* p = "WRGYOB"; *p; ++p) {
            if (counts[static_cast<unsigned char>(*p)] != 9) {
                check(false, std::string("colour count wrong for ") + *p);
                all_ok = false;
            }
        }
        if (!all_ok) break;
    }
    if (all_ok) check(true, "200 random encode/decode round-trips");

    // An impossible cube must be rejected rather than silently mangled.
    std::string bad = TARGET_YG;
    bad[0] = 'R';  // duplicates a corner, breaks the piece set
    CubeState dummy;
    check(!enc.encode(bad, dummy), "illegal sticker set rejected");
}

// Fixture states captured from the TypeScript engine: scramble -> the facelet
// string applyNotationToFacelets() produces. If the C++ engine ever drifts
// from the TS one, these break first.
struct Fixture { const char* scramble; const char* facelets; };
extern const std::vector<Fixture> TS_FIXTURES;

static void testAgainstTypeScript() {
    std::printf("cross-check against the TypeScript engine\n");
    CubieEncoder enc(TARGET_YG);

    int mismatches = 0;
    for (const Fixture& f : TS_FIXTURES) {
        CubeState s = CubeState::solved();
        s.applyNotation(f.scramble);
        std::string got = enc.decode(s);
        if (got != f.facelets) {
            if (mismatches < 3) {
                std::printf("  FAIL: %s\n    ts  = %s\n    cpp = %s\n",
                            f.scramble, f.facelets, got.c_str());
            }
            ++mismatches;
        }
    }
    // %lu with an explicit cast rather than %zu: MinGW's printf does not
    // implement the z length modifier.
    const unsigned long total = static_cast<unsigned long>(TS_FIXTURES.size());
    ++g_checks;
    if (mismatches) {
        ++g_failures;
        std::printf("  FAIL: %d/%lu fixtures disagree with the TS engine\n", mismatches, total);
    } else {
        std::printf("  all %lu TS fixtures match move-for-move\n", total);
    }
}

struct Stages { int cross = 0, f2l = 0, oll = 0, pll = 0; };

static bool solveAndVerify(const std::string& scramble, bool fa, bool oa, bool pa,
                           int& out_moves, std::string& err,
                           Stages* stages = nullptr, double* ms = nullptr) {
    CubieEncoder enc(TARGET_YG);
    CubeState scrambled = CubeState::solved();
    scrambled.applyNotation(scramble);
    const std::string facelets = enc.decode(scrambled);

    const auto t0 = std::chrono::steady_clock::now();
    SolveOutput r = solveFacelets(facelets, TARGET_YG, fa, oa, pa);
    const auto t1 = std::chrono::steady_clock::now();
    if (ms) *ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    if (!r.success) { err = r.errorMessage; return false; }

    if (stages) {
        stages->cross = static_cast<int>(parseNotation(r.crossMoves).size());
        stages->f2l = static_cast<int>(parseNotation(r.f2lMoves).size());
        stages->oll = static_cast<int>(parseNotation(r.ollMoves).size());
        stages->pll = static_cast<int>(parseNotation(r.pllMoves).size());
    }

    std::string all = r.crossMoves;
    for (const std::string* p : {&r.f2lMoves, &r.ollMoves, &r.pllMoves}) {
        if (!p->empty()) { if (!all.empty()) all += " "; all += *p; }
    }

    std::vector<Move> mv = parseNotation(all);
    out_moves = static_cast<int>(mv.size());

    CubeState check_state = scrambled;
    check_state.applyMoves(mv);
    if (!isSolved(check_state)) { err = "solution does not solve the cube"; return false; }
    return true;
}

static void testSolves(const char* label, bool fa, bool oa, bool pa, int trials) {
    std::printf("full solves - %s\n", label);

    static const char* FIXED[] = {
        "R U R' U' F2 L D B' R2",
        "D2 F' L2 B R U2 L' D R' F U B2",
        "U F2 R' D' L B U2 R F' D2 L2 U'",
        "B' R2 U L' F D' B2 U2 R L F2 D",
        "L U' B2 D R' F L2 B U' R2 D' F'",
        "F R U' L' D2 B R2 U F2 L D' B'",
        "R2 D L' B' U F2 R D2 L U' B F",
        "U' L2 B R D' F' U2 L B2 R' D F2",
        // Superflip: every edge in place but flipped, a classic worst case.
        "U R2 F B R B2 R U2 L B2 R U' D' R2 F R' L B2 U2 F2",
    };

    long total = 0, worst = 0;
    int solved = 0, attempted = 0;
    Stages sum;
    double ms_total = 0, ms_worst = 0;

    auto attempt = [&](const std::string& scr, const char* kind) {
        int n = 0; std::string err; Stages st; double ms = 0;
        ++attempted;
        if (solveAndVerify(scr, fa, oa, pa, n, err, &st, &ms)) {
            ++solved; total += n; if (n > worst) worst = n;
            sum.cross += st.cross; sum.f2l += st.f2l; sum.oll += st.oll; sum.pll += st.pll;
            ms_total += ms; if (ms > ms_worst) ms_worst = ms;
        } else {
            check(false, std::string(kind) + " scramble '" + scr + "': " + err);
        }
    };

    for (const char* s : FIXED) attempt(s, "fixed");

    Mulberry32 rng(2024);
    for (int i = 0; i < trials; ++i) attempt(randomScramble(rng, 25), "random");

    check(solved == attempted, std::string(label) + ": every scramble solved");
    if (solved) {
        const double d = solved;
        std::printf("  %d/%d solved | mean %.1f moves (worst %ld)"
                    " | cross %.1f  F2L %.1f  OLL %.1f  PLL %.1f\n",
                    solved, attempted, total / d, worst,
                    sum.cross / d, sum.f2l / d, sum.oll / d, sum.pll / d);
        std::printf("  mean %.1f ms per solve (worst %.1f ms)\n", ms_total / d, ms_worst);
    }
}

namespace rubiks {
extern int PLL_ONE_LOOK_DEPTH;
extern long g_pll_one_look_hits;
extern long g_pll_one_look_misses;
}

int main(int argc, char** argv) {
    if (argc > 1) rubiks::PLL_ONE_LOOK_DEPTH = std::atoi(argv[1]);
    const int trials = argc > 2 ? std::atoi(argv[2]) : 40;
    std::printf("PLL one-look depth = %d, random trials = %d\n\n",
                rubiks::PLL_ONE_LOOK_DEPTH, trials);

    testMoveEngine();
    testFaceletRoundTrip();
    testAgainstTypeScript();
    testSolves("Full CFOP (all advanced)", true, true, true, trials);
    std::printf("  PLL one-look: %ld hit, %ld fell back to two-look\n",
                rubiks::g_pll_one_look_hits, rubiks::g_pll_one_look_misses);
    testSolves("Beginner (all decomposed)", false, false, false, 20);
    testSolves("Hybrid (adv F2L, beginner LL)", true, false, false, 10);

    std::printf("\n%d checks, %d failures\n", g_checks, g_failures);
    return g_failures ? 1 : 0;
}
