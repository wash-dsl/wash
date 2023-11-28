#pragma once

#include <string>
#include <vector>
#include <functional>
#include <iostream>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <variant>

using Real    = double;
using KeyType = uint64_t;

template<class... T, class... Separators>
void writeAscii(size_t firstIndex, size_t lastIndex, const std::string& path, bool append,
                const std::vector<std::variant<T*...>>& fields, Separators&&... separators);

void printHelp(char* binName);
void writeColumns1D(const std::string& path);

class SedovComputer {
public:
    static double rho_shock;
    static double p_shock;
    static double vel_shock;
    static double u_shock;
    static double cs_shock;
    static double sedovSol(const size_t          dim,     // geometry factor: 1=planar, 2=cylindircal, 3=spherical
                           const double          time,    // temporal point where solution is desired [seconds]
                           const double          eblast,  // energy of blast in the wave front [erg]
                           const double          omega_i, // density power law exponent in 'rho = rho0 * r**(-omega)'
                           const double          gamma_i, // gamma law equation of state
                           const double          rho0,    // ambient density g/cm**3 in 'rho = rho0 * r**(-omega)'
                           const double          u0,      // ambient internal energy [erg/g]
                           const double          p0,      // ambient pressure [erg/cm**3]
                           const double          vel0,    // ambient material speed [cm/s]
                           const double          cs0,     // ambient sound speed [cm/s]
                           const std::vector<double>& r,       // out: spatial points where solution is desired [cm]
                           std::vector<double>&       rho,     // out: density  [g/cm**3]
                           std::vector<double>&       p,       // out: presssure [erg/cm**3]
                           std::vector<double>&       u,       // out: specific internal energy [erg/g]
                           std::vector<double>&       vel,     // out: velocity [cm/s]
                           std::vector<double>&       cs);           // out: sound speed [cm/s]

private:
    // Constants
    static inline const double eps = 1.e-10;    // eps controls the integration accuracy, don't get too greedy or the
                                                // number of function evaluations required kills.
    static inline const double eps2   = 1.e-30; // eps2 controls the root find accuracy
    static inline const double osmall = 1.e-4;  // osmall controls the size of transition regions

    // Private global variables
    static double xgeom, omega, gamma;               //
    static double gamm1, gamp1, gpogm, xg2;          //
    static bool   lsingular, lstandard, lvacuum;     //
    static bool   lomega2, lomega3;                  //
    static double a0, a1, a2, a3, a4, a5;            //
    static double a_val, b_val, c_val, d_val, e_val; //
    static double rwant, vwant;                      //
    static double r2, v0, vv, rvv;                   //
    static double gam_int;                           //

    static void sedov_funcs(const double v,      // Similarity variable v
                            double&      l_fun,  // out: l_fun is book's zeta
                            double&      dlamdv, // out: l_fun derivative
                            double&      f_fun,  // out: f_fun is book's V
                            double&      g_fun,  // out: g_fun is book's D
                            double&      h_fun);      // out: h_fun is book's P

    static double efun01(const double v); //

    static double efun02(const double v); //

    static double sed_v_find(const double v); //

    static double sed_r_find(const double r); //

    static void midpnt(const size_t                   n,    //
                       std::function<double(const double)> func, //
                       const double                   a,    //
                       const double                   b,    //
                       double&                        s);                          //

    static double midpowl_func(std::function<double(const double)> funk, //
                               const double                   x,    //
                               const double                   aa);                    //

    static void midpowl(const size_t                   n,    //
                        std::function<double(const double)> funk, //
                        const double                   aa,   //
                        const double                   bb,   //
                        double&                        s);                          // out:

    static double midpowl2_func(std::function<double(const double)> funk, //
                                const double                   x,    //
                                const double                   aa);                    //

    static void midpowl2(const size_t                   n,    //
                         std::function<double(const double)> funk, //
                         const double                   aa,   //
                         const double                   bb,   //
                         double&                        s);                          // out:

    static void polint(double*      xa, //
                       double*      ya, //
                       const size_t n,  //
                       const double x,  //
                       double&      y,  // out:
                       double&      dy);     // out:

    static void
    qromo(std::function<double(const double)> func,    //
          const double a,       //
          const double b,       //
          const double eps,     //
          double& ss,      //
          std::function<void(const size_t, std::function<double(const double)>, const double, const double, double&)> choose); //

    static double zeroin(
        const double                   ax, // Left endpoint of initial interval
        const double                   bx, // Right endpoint of initial interval
        std::function<double(const double)> f,  // Function subprogram which evaluates f(x) for any x in the interval [ax,bx]
        const double tol);                 // Desired length of the interval of uncertainty of the final result (>= 0.)
};