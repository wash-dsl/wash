#include "configurations.hpp"

namespace ws2st {

namespace refactor {

namespace config {

    void writeWONEParticleDataInitialiser(const WashOptions& opts) {
        std::string output_str =
            "#include \"particle_data.hpp\" \n"
            "namespace wash {\n";

        // always x, y, z for cornerstone?
        for (auto dim = 0; dim < 3; dim++) {
            output_str += "std::vector<double> vector_force_pos_" + std::to_string(dim) + ";\n"; 
        }

        for (auto dim = 0; dim < program_meta->simulation_dimension; dim++) {
            output_str += "std::vector<double> vector_force_vel_" + std::to_string(dim) + ";\n"; 
        }

        for (auto dim = 0; dim < program_meta->simulation_dimension; dim++) {
            output_str += "std::vector<double> vector_force_acc_" + std::to_string(dim) + ";\n"; 
        }

        output_str += "std::vector<double> scalar_force_mass;\n"
            "std::vector<double> scalar_force_density;\n"
            "std::vector<double> scalar_force_smoothing_length;\n";

        output_str += ws2st::refactor::forces::getForceDeclarationSource();

        output_str += ws2st::refactor::variables::getVariableDefinitionSource();

        output_str +=
            "void _initialise_particle_data(size_t particlec) {\n";

        // always x, y, z for cornerstone?
        for (auto dim = 0; dim < 3; dim++) {
            output_str += "wash::vector_force_pos_" + std::to_string(dim) + " = std::vector<double>(particlec);\n"; 
        }

        for (auto dim = 0; dim < program_meta->simulation_dimension; dim++) {
            output_str += "wash::vector_force_vel_" + std::to_string(dim) + " = std::vector<double>(particlec);\n"; 
        }

        for (auto dim = 0; dim < program_meta->simulation_dimension; dim++) {
            output_str += "wash::vector_force_acc_" + std::to_string(dim) + " = std::vector<double>(particlec);\n"; 
        }

        output_str += "    wash::scalar_force_mass = std::vector<double>(particlec);\n"
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

    RefactoringToolConfiguration wone_rules = {
        // {
        //     &AllFiles,
        //     // Detect kernels
        //     WashRefactoringAction(&dependency_detection::AddForceKernelMatcher, &dependency_detection::RegisterForceKernel),
        // },
        // {
        //     &AllFiles,
        //     // Detect force dependencies
        //     WashRefactoringAction(&dependency_detection::ForceAssignmentInFunction, &dependency_detection::RegisterForceAssignment),
        //     WashRefactoringAction(&dependency_detection::PosAssignmentInFunction, &dependency_detection::RegisterPosAssignment),
        //     WashRefactoringAction(&dependency_detection::VelAssignmentInFunction, &dependency_detection::RegisterVelAssignment),
        //     WashRefactoringAction(&dependency_detection::AccAssignmentInFunction, &dependency_detection::RegisterAccAssignment),

        //     WashRefactoringAction(&dependency_detection::ForceReadInFunction, &dependency_detection::RegisterForceRead),
        //     WashRefactoringAction(&dependency_detection::PosReadInFunction, &dependency_detection::RegisterPosRead),
        //     WashRefactoringAction(&dependency_detection::VelReadInFunction, &dependency_detection::RegisterVelRead),
        //     WashRefactoringAction(&dependency_detection::AccReadInFunction, &dependency_detection::RegisterAccRead),
        // },

        // // 0th pass: Information gathering about the simulation
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
            WashRefactoringAction(&forces::GetForceVectorMatcher, forces::HandleGetForceVectorWithCornerstone),
            // Calls to get pre-defined particle properties
            WashRefactoringAction(&forces::GetPosMatcher, forces::HandleGetPosWithCornerstone),
            WashRefactoringAction(&forces::GetVelMatcher, forces::HandleGetVelWithCornerstone),
            WashRefactoringAction(&forces::GetAccMatcher, forces::HandleGetAccWithCornerstone),
        
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
            WashRefactoringAction(&forces::SetForceVectorMatcher, forces::HandleSetForceVectorWithCornerstone),
            // Calls to set pre-defined particle properties
            WashRefactoringAction(&forces::SetPosMatcher, forces::HandleSetPosWithCornerstone),
            WashRefactoringAction(&forces::SetVelMatcher, forces::HandleSetVelWithCornerstone),
            WashRefactoringAction(&forces::SetAccMatcher, forces::HandleSetAccWithCornerstone),
        
            WashRefactoringAction(&forces::SetDensityMatcher, forces::HandleSetDensity),
            WashRefactoringAction(&forces::SetMassMatcher, forces::HandleSetMass),
            WashRefactoringAction(&forces::SetSmoothingLengthMatcher, forces::HandleSetSmoothingLength),

            // Calls to set a variable
            WashRefactoringAction(&variables::SetVariableMatcher, &variables::HandleSetVariable),
        },
        {
            WashComputationAction(&writeWONEParticleDataInitialiser)
        }
    };

}

}

}