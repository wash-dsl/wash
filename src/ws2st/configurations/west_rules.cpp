#include "configurations.hpp"

namespace ws2st {

namespace refactor {

namespace config {

    void writeWESTParticleDataInitialiser(const WashOptions& opts) {
        std::string output_str =
            "#include \"particle_data.hpp\" \n"
            "namespace wash {\n"
            "std::vector<SimulationVecT> vector_force_pos;\n"
            "std::vector<SimulationVecT> vector_force_vel;\n"
            "std::vector<SimulationVecT> vector_force_acc;\n"
            "std::vector<double> scalar_force_mass;\n"
            "std::vector<double> scalar_force_density;\n"
            "std::vector<double> scalar_force_smoothing_length;\n";

        output_str += ws2st::refactor::forces::getForceDeclarationSource();

        output_str += ws2st::refactor::variables::getVariableDefinitionSource();

        output_str +=
            "void _initialise_particle_data(size_t particlec) {\n"
            "    wash::vector_force_pos = std::vector<SimulationVecT>(particlec);\n"
            "    wash::vector_force_vel = std::vector<SimulationVecT>(particlec);\n"
            "    wash::vector_force_acc = std::vector<SimulationVecT>(particlec);\n"
            "    wash::scalar_force_mass = std::vector<double>(particlec);\n"
            "    wash::scalar_force_density = std::vector<double>(particlec);\n"
            "    wash::scalar_force_smoothing_length = std::vector<double>(particlec);\n";

        output_str += ws2st::refactor::forces::getForceInitialisationSource();

        output_str += " } }";

        std::ios_base::openmode mode = std::ofstream::out;
        std::string path = opts.temp_path + "/particle_data.cpp";
        std::ofstream outfile(path.c_str(), mode);

        outfile << output_str.c_str() << std::endl;

        outfile.close();
    }

    RefactoringToolConfiguration west_rules = {
        // 0th pass: Information gathering about the simulation
        {
            &AllFiles,
            // Register Scalar/Vector forces with the simulation
            WashRefactoringAction(&forces::AddForceVectorMatcher, forces::HandleRegisterForcesVector),
            WashRefactoringAction(&forces::AddForceScalarMatcher, forces::HandleRegisterForcesScalar),
            
            // Register Variables with the simulation
            WashRefactoringAction(&variables::RegisterVariableMatcher, &variables::HandleRegisterVariable),
            WashRefactoringAction(&variables::RegisterVariableNoInitMatcher, &variables::HandleRegisterVariable),
            WashRefactoringAction(&meta::SetDimensionMatcher, &meta::HandleSetDimension),
        },
        // 1st pass: registration, gets
        {   
            &AllFiles,
            // Rewrite IO functions which inspect the list of forces/force names
            WashRefactoringAction(&meta::DefineForceAccessFnMatcher, &meta::DefineForceAccessFns),
            // Calls to get a force
            WashRefactoringAction(&forces::GetForceScalarMatcher, forces::HandleGetForceScalar),
            WashRefactoringAction(&forces::GetForceVectorMatcher, forces::HandleGetForceVector),
            // Calls to get pre-defined particle properties
            WashRefactoringAction(&forces::GetPosMatcher, forces::HandleGetPos),
            WashRefactoringAction(&forces::GetVelMatcher, forces::HandleGetVel),
            WashRefactoringAction(&forces::GetAccMatcher, forces::HandleGetAcc),
        
            WashRefactoringAction(&forces::GetDensityMatcher, forces::HandleGetDensity),
            WashRefactoringAction(&forces::GetMassMatcher, forces::HandleGetMass),
            WashRefactoringAction(&forces::GetSmoothingLengthMatcher, forces::HandleGetSmoothingLength),

            // Calls to get a variable
            WashRefactoringAction(&variables::GetVariableMatcher, &variables::HandleGetVariable),
            WashRefactoringAction(&variables::GetVariableRefMatcher, &variables::HandleGetVariableRef),
            // Replacement to add the force definitions
            WashRefactoringAction(&forces::InsertForcesDefinitionMatcher, &forces::HandleInsertForcesDefinition),
            WashRefactoringAction(&variables::InsertVariablesDeclarationMatcher, &variables::HandleInsertVariablesDeclaration),

            WashRefactoringAction(&meta::SimulationVecTMatcher, &meta::HandleSimulationVecTMatcher)
        },
        // 2nd pass: set calls
        {
            &AllFiles,
            // Calls to set a force
            WashRefactoringAction(&forces::SetForceScalarMatcher, forces::HandleSetForceScalar),
            WashRefactoringAction(&forces::SetForceVectorMatcher, forces::HandleSetForceVector),
            // Calls to set pre-defined particle properties
            WashRefactoringAction(&forces::SetPosMatcher, forces::HandleSetPos),
            WashRefactoringAction(&forces::SetVelMatcher, forces::HandleSetVel),
            WashRefactoringAction(&forces::SetAccMatcher, forces::HandleSetAcc),
        
            WashRefactoringAction(&forces::SetDensityMatcher, forces::HandleSetDensity),
            WashRefactoringAction(&forces::SetMassMatcher, forces::HandleSetMass),
            WashRefactoringAction(&forces::SetSmoothingLengthMatcher, forces::HandleSetSmoothingLength),

            // Calls to set a variable
            WashRefactoringAction(&variables::SetVariableMatcher, &variables::HandleSetVariable),
        },
        {
            // Final thing is to write the particle data initialiser into particle_data.cpp
            &NoFiles, // No need to actually run a tool here
            WashComputationAction(&writeWESTParticleDataInitialiser)
        }
    };

}

}

}