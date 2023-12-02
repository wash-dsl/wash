/**
 * @file main.cpp
 * @author Jose A. Escartin <ja.escartin@gmail.com>
 * @brief This file is based on the analytical solution presented in SPH-EXA <https://github.com/unibas-dmi-hpc/SPH-EXA>
 * @date 2023-11-28
 *
 * This program generates the analytical sedov solution based in the time and the initial conditions
 */
#include "sedov_computer.hpp"

#include <optional>
#include <getopt.h>

int main(int argc, char** argv) {
    // const ArgParser parser(argc, (const char**)argv);

    // if (parser.exists("-h") || parser.exists("--h") || parser.exists("-help") || parser.exists("--help"))
    // {
    //     printHelp(argv[0]);
    //     exit(EXIT_SUCCESS);
    // }

    std::optional<std::string> argTime;
    std::optional<std::string> argOutDir;
    int argNormalize;
    std::optional<std::string> argOut;

    static struct option options[] {
        {"time", required_argument, nullptr, 't'},
        {"outDir", required_argument, nullptr, 'd'},
        {"normalize", no_argument, &argNormalize, 1},
        {"out", required_argument, nullptr, 'o'},
        {0, 0, 0, 0}
    };

    int optidx;
    int c;
    while (true) {
        c = getopt_long(argc, argv, "t:d:no:", options, &optidx);

        if (c == -1) break;

        switch (c) {
            case 't':
                argTime.emplace(optarg);
                break;
            case 'd':
                argOutDir.emplace(optarg);
                break;
            case 'o': 
                argOut.emplace(optarg);
                break;
            default:
                break;
        }
    }

    // Get command line parameters
    // const double      time      = parser.get<double>("--time", 0.);
    // const std::string outDir    = parser.get<std::string>("--outDir", "./");
    // const bool        normalize = parser.exists("--normalize");
    const double time = std::stod(argTime.value_or("0.2"));
    const std::string outDir = argOutDir.value_or("./out/sedov_sol/");
    const bool normalize = argNormalize == 1;

    // Get time without rounding
    std::ostringstream time_long;
    time_long << time;
    std::string time_str = time_long.str();

    // const string solFile =
    //     parser.exists("--out") ? parser.get("--out") : outDir + "sedov_solution_" + time_str + ".dat";
    // const std::string solFile = outDir + "sedov_sol_" + time_str + ".dat";
    const std::string solFile = argOut.value_or(outDir + "sedov_sol_" + time_str + ".dat");

    // // Calculate and write theoretical solution profile in one dimension
    // auto         constants = sedovConstants();
    // const size_t dim       = constants["dim"];
    // const double r0        = constants["r0"];
    // const double r1        = constants["r1"];
    // const double eblast    = constants["energyTotal"];
    // const double gamma     = constants["gamma"];
    // const double omega     = constants["omega"];
    // const double rho0      = constants["rho0"];
    // const double u0        = constants["u0"];
    // const double p0        = constants["p0"];
    // const double vr0       = constants["vr0"];
    // const double cs0       = constants["cs0"];

    const size_t dim = 3;
    const double r0 = 0.0;
    const double r1 = 0.5;
    const double eblast = 1.0;
    const double gamma = 5.0 / 3.0;
    const double omega = 0.0;
    const double rho0 = 1.0;
    const double u0 = 1e-8;
    const double p0 = 0.0;
    const double vr0 = 0.0;
    const double cs0 = 0.0;

    double shockFront;
    {
        std::vector<double> rDummy(1, 0.1);
        std::vector<Real> rho(1), p(1), u(1), vel(1), cs(1);
        shockFront = SedovComputer::sedovSol(dim, time, eblast, omega, gamma, rho0, u0, p0, vr0, cs0, rDummy, rho, p, u,
                                             vel, cs);
    }

    // Set the positions for calculating the solution
    size_t nSteps = 100000;
    size_t nSamples = nSteps + 2;
    std::vector<double> rSol(nSamples);

    const double rMax = 2. * r1;
    const double rStep = (rMax - r0) / nSteps;

    for (size_t i = 0; i < nSteps; i++) {
        rSol[i] = (r0 + (0.5 * rStep) + (i * rStep));
    }
    rSol[nSamples - 2] = shockFront;
    rSol[nSamples - 1] = shockFront + 1e-7;
    std::sort(std::begin(rSol), std::end(rSol));

    // analytical solution output
    std::vector<Real> rho(nSamples), p(nSamples), u(nSamples), vel(nSamples), cs(nSamples);

    // Calculate theoretical solution
    SedovComputer::sedovSol(dim, time, eblast, omega, gamma, rho0, u0, p0, vr0, cs0, rSol, rho, p, u, vel, cs);

    if (normalize) {
        std::for_each(std::begin(rho), std::end(rho), [](auto& val) { val /= SedovComputer::rho_shock; });
        std::for_each(std::begin(u), std::end(u), [](auto& val) { val /= SedovComputer::u_shock; });
        std::for_each(std::begin(p), std::end(p), [](auto& val) { val /= SedovComputer::p_shock; });
        std::for_each(std::begin(vel), std::end(vel), [](auto& val) { val /= SedovComputer::vel_shock; });
        std::for_each(std::begin(cs), std::end(cs), [](auto& val) { val /= SedovComputer::cs_shock; });
    }

    writeColumns1D(solFile);
    writeAscii<Real>(0, nSteps, solFile, true, {rSol.data(), rho.data(), u.data(), p.data(), vel.data(), cs.data()},
                     std::setw(16), std::setprecision(7), std::scientific);

    std::cout << "Created solution file: '" << solFile << std::endl;

    return EXIT_SUCCESS;
}

void printHelp(char* binName) {
    printf("\nUsage:\n\n");
    printf("%s [OPTIONS]\n", binName);
    printf("\nWhere possible options are:\n\n");

    printf("\t--time     NUM  \t\t Time where the solution is calculated (secs) [0.]\n\n");

    printf(
        "\t--outPath  PATH \t\t Path to directory where output will be saved [./].\
                \n\t\t\t\t Note that directory must exist and be provided with ending slash.\
                \n\t\t\t\t Example: --outDir /home/user/folderToSaveOutputFiles/\n\n");
}

void writeColumns1D(const std::string& path) {
    std::ofstream out(path);

    out << std::setw(16) << "#           01:r"     // Column : position 1D     (Real value)
        << std::setw(16) << "02:rho"               // Column : density         (Real value)
        << std::setw(16) << "03:u"                 // Column : internal energy (Real value)
        << std::setw(16) << "04:p"                 // Column : pressure        (Real value)
        << std::setw(16) << "05:vel"               // Column : velocity 1D     (Real value)
        << std::setw(16) << "06:cs" << std::endl;  // Column : sound speed     (Real value)

    out.close();
}

template <class... T, class... Separators>
void writeAscii(size_t firstIndex, size_t lastIndex, const std::string& path, bool append,
                const std::vector<std::variant<T*...>>& fields, Separators&&... separators) {
    std::ios_base::openmode mode;
    if (append) {
        mode = std::ofstream::app;
    } else {
        mode = std::ofstream::out;
    }

    std::ofstream dumpFile(path, mode);

    if (dumpFile.is_open()) {
        for (size_t i = firstIndex; i < lastIndex; ++i) {
            for (auto field : fields) {
                [[maybe_unused]] std::initializer_list<int> list{(dumpFile << separators, 0)...};
                std::visit([&dumpFile, i](auto& arg) { dumpFile << arg[i]; }, field);
            }
            dumpFile << std::endl;
        }
    } else {
        throw std::runtime_error("Can't open file at path: " + path);
    }

    dumpFile.close();
}