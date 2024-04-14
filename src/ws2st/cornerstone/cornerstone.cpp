#include "cornerstone.hpp"

namespace ws2st {
    
    namespace refactor {

        namespace cornerstone {

            DeclarationMatcher DataSetupDecl = traverse(TK_IgnoreUnlessSpelledInSource, 
                cxxRecordDecl(hasName("_wash_data_setup")).bind("decl")
            );

            void HandleSetupDataWithCornerstone(const MatchFinder::MatchResult& Result, Replacements& Replace) {
                const auto decl = Result.Nodes.getNodeAs<CXXRecordDecl>("decl");

                std::string output_str;
                std::vector<std::string> scalars = program_meta->scalar_force_list;
                scalars.push_back("id");
                scalars.push_back("mass");
                scalars.push_back("density");
                scalars.push_back("smoothing_length");
                std::vector<std::string> vectors = program_meta->vector_force_list;
                vectors.push_back("pos");
                vectors.push_back("vel");
                vectors.push_back("acc");

                for (auto scalar : scalars) {
                    output_str += "wash::scalar_force_" + scalar + ".resize(local_count);\n";
                }
                for (auto vector : vectors) {
                    for (auto dim = 0; dim < program_meta->simulation_dimension; dim++) {
                        output_str += "wash::vector_force_" + vector + "_" + std::to_string(dim) + ".resize(local_count);\n";
                    }
                    if (vector == "pos" && program_meta->simulation_dimension == 2) {
                        output_str += "wash::vector_force_pos_2.resize(local_count);\n";
                        // "for (unsigned i = 0; i < local_count; i++) {\n\t"
                        // "wash::vector_force_2[i] = "
                        // "}\n";
                    }
                }

                output_str += "for (unsigned i = 0; i < local_count; i++) {\n\t"
                "wash::scalar_force_id[i] = first_id + i;\n"
                "}\n";

                auto Err = Replace.add(Replacement(
                    *Result.SourceManager, CharSourceRange::getTokenRange(decl->getSourceRange()), output_str));

                if (Err) {
                    std::cout << llvm::toString(std::move(Err)) << std::endl;
                    throw std::runtime_error("Error handling a match callback.");
                } else {
                    std::cout << "Inserted Setup Data code." << std::endl;
                }
            }

            DeclarationMatcher ParticleRecalculateNeighbours = traverse(TK_IgnoreUnlessSpelledInSource, 
                cxxRecordDecl(hasName("_wash_recalc_neighbours")).bind("decl")
            );
            
            void HandleRecalculateNeighboursWithCornerstone(const MatchFinder::MatchResult& Result, Replacements& Replace) {
                const auto decl = Result.Nodes.getNodeAs<CXXRecordDecl>("decl");

                std::string output_str = "unsigned recalculate_neighbours(const Particle& p, unsigned max_count) {\n\t"
                "unsigned count = cstone::findNeighbors("
                "p, \n\t\t"
                "wash::vector_force_pos_0.data(), wash::vector_force_pos_1.data(), wash::vector_force_pos_2.data(),"
                "wash::scalar_force_smoothing_length.data(),\n\t\t"
                "(*domain).octreeProperties().nsView(), (*domain).box(), max_count,\n\t\t"
                "neighbors_data.data() + p * neighbors_max\n\t"
                ");\n\t"
                "neighbors_cnt[p] = std::min(count, neighbors_max);\n\t"
                "return count;\n}\n"; 
                // "std::cout << \"recalc p \" << local_idx << \" count \" << count << std::endl;\n\t"
                
                auto Err = Replace.add(Replacement(
                    *Result.SourceManager, CharSourceRange::getTokenRange(decl->getSourceRange()), output_str));

                if (Err) {
                    std::cout << llvm::toString(std::move(Err)) << std::endl;
                    throw std::runtime_error("Error handling a match callback.");
                } else {
                    std::cout << "Inserted recalculate neighbours implementation." << std::endl;
                }
            }

            DeclarationMatcher SyncDomainCall = traverse(TK_IgnoreUnlessSpelledInSource, 
                cxxRecordDecl(hasName("_wash_sync_domain")).bind("decl")
            );

            void HandleSyncDomainWithCornerstone(const MatchFinder::MatchResult& Result, Replacements& Replace) {
                const auto decl = Result.Nodes.getNodeAs<CXXRecordDecl>("decl");

                std::vector<std::string> scalars = program_meta->scalar_force_list;
                scalars.push_back("id");
                scalars.push_back("mass");
                scalars.push_back("density");
                scalars.push_back("smoothing_length");
                std::vector<std::string> vectors = program_meta->vector_force_list;
                vectors.push_back("pos");
                vectors.push_back("vel");
                vectors.push_back("acc");

                std::string particle_properties = "std::tie(";

                bool not_first = false;
                for (auto scalar : scalars) {
                    if (scalar == "smoothing_length") continue;
                    if (not_first) {
                        particle_properties += ",";
                    }
                    particle_properties += "wash::scalar_force_" + scalar;
                    not_first = true;
                }
                for (auto vector : vectors) {
                    if (vector == "pos") continue;
                    for (auto dim = 0; dim < program_meta->simulation_dimension; dim++) {
                        particle_properties += ",wash::vector_force_" + vector + "_" + std::to_string(dim);
                    }
                }
                particle_properties += ")";

                KernelDependencies* neighbour_search_dependencies = program_meta->kernels_dependency_map.at(program_meta->neighbour_kernel).get();
                
                std::vector<std::string> reads = neighbour_search_dependencies->reads_from;
                reads.push_back("id");

                std::vector<std::string> writes = neighbour_search_dependencies->writes_to;
                writes.push_back("id");

                std::string exchange_before = dependency_detection::RunHaloExchange(neighbour_search_dependencies->reads_from);
                exchange_before.erase(0, 9);
                exchange_before = "domain" + exchange_before;

                std::string exchange_after  = dependency_detection::RunHaloExchange(neighbour_search_dependencies->writes_to);
                exchange_after.erase(0, 9);
                exchange_after = "domain" + exchange_after;
                

                std::string output_str = 
                "domain.sync("
                "keys,"
                "wash::vector_force_pos_0, wash::vector_force_pos_1, wash::vector_force_pos_2, wash::scalar_force_smoothing_length," 
                + particle_properties +
                ", std::tie(s1, s2, s3));\n"
                "start_idx = domain.startIndex();\n"
                "end_idx = domain.endIndex();\n"
                "domain.exchangeHalos(std::tie(wash::scalar_force_id), s1, s2);\n"
                "neighbors_cnt.resize(domain.nParticlesWithHalos());\n"
                "neighbors_data.resize(domain.nParticlesWithHalos() * neighbors_max);\n"
                + exchange_before +
                "#pragma omp parallel for\n"
                "for (unsigned i = start_idx; i < end_idx; i++) {\n"
                "    wash::Particle p(i);\n"
                "    neighbors_kernel(p);\n"
                "}\n"
                + exchange_after;

                auto Err = Replace.add(Replacement(
                    *Result.SourceManager, CharSourceRange::getTokenRange(decl->getSourceRange()), output_str));

                if (Err) {
                    std::cout << llvm::toString(std::move(Err)) << std::endl;
                    throw std::runtime_error("Error handling a match callback.");
                } else {
                    std::cout << "Inserted domain sync code." << std::endl;
                }
            }

            DeclarationMatcher ExchangeAllHalos = traverse(TK_IgnoreUnlessSpelledInSource, 
                cxxRecordDecl(hasName("_wash_exchange_all_halos")).bind("decl")
            );

            void HandleExchangeAllHalosWithCornerstone(const MatchFinder::MatchResult& Result, Replacements& Replace) {
                const auto decl = Result.Nodes.getNodeAs<CXXRecordDecl>("decl");
                std::vector<std::string> scalars = program_meta->scalar_force_list;
                scalars.push_back("id");
                scalars.push_back("mass");
                scalars.push_back("density");
                scalars.push_back("smoothing_length");
                std::vector<std::string> vectors = program_meta->vector_force_list;
                vectors.push_back("pos");
                vectors.push_back("vel");
                vectors.push_back("acc");

                std::string particle_properties = "std::tie(";
                bool not_first = false;
                for (auto scalar : scalars) {
                    if (not_first) {
                        particle_properties += ",";
                    }
                    particle_properties += "wash::scalar_force_" + scalar;
                    not_first = true;
                }
                for (auto vector : vectors) {
                    for (auto dim = 0; dim < program_meta->simulation_dimension; dim++) {
                        particle_properties += ",wash::vector_force_" + vector + "_" + std::to_string(dim);
                    }
                    if (vector == "pos" && program_meta->simulation_dimension == 2) {
                        particle_properties += ",wash::vector_force_pos_2";
                    }
                }
                particle_properties += ")";
                // domain.exchangeHalos(wash::make_tuple<std::vector<double>, MAX_FORCES>(force_data), s1, s2);
                std::string output_str = "(*domain).exchangeHalos(" + particle_properties + ", s1, s2);\n";

                auto Err = Replace.add(Replacement(
                    *Result.SourceManager, CharSourceRange::getTokenRange(decl->getSourceRange()), output_str));

                if (Err) {
                    std::cout << llvm::toString(std::move(Err)) << std::endl;
                    throw std::runtime_error("Error handling a match callback.");
                } else {
                    std::cout << "Inserted exchange all halos." << std::endl;
                }
            }


            DeclarationMatcher ForceKernelExec = traverse(TK_IgnoreUnlessSpelledInSource, 
                cxxRecordDecl(hasName("_wash_force_kernel_exec")).bind("decl")
            );

            void HandleForceKernelExec(const MatchFinder::MatchResult& Result, Replacements& Replace) {
                const auto decl = Result.Nodes.getNodeAs<CXXRecordDecl>("decl");

                std::string output_str = "auto begin = neighbors_data.cbegin() + i * neighbors_max;\n"
                "auto end = begin + count;\n"
                "func(p, begin, end);\n";

                auto Err = Replace.add(Replacement(*Result.SourceManager, CharSourceRange::getTokenRange(decl->getSourceRange()), output_str));
                if (Err) {
                    std::cout << llvm::toString(std::move(Err)) << std::endl;
                    throw std::runtime_error("Error writing new force kernel exec code.");
                } else {
                    std::cout << "Inserted force kernel execution code" << std::endl;
                }
            }

            DeclarationMatcher ParticleClassMatcher = traverse(TK_IgnoreUnlessSpelledInSource, 
                cxxRecordDecl(hasName("Particle")).bind("decl")
            );

            void HandleRemoveParticleClass(const MatchFinder::MatchResult& Result, Replacements& Replace) {
                const auto decl = Result.Nodes.getNodeAs<CXXRecordDecl>("decl");

                std::string output_str = "using Particle = unsigned;\n"
                "unsigned recalculate_neighbours(const Particle& p, unsigned max_count);";

                auto Err = Replace.add(Replacement(*Result.SourceManager, CharSourceRange::getTokenRange(decl->getSourceRange()), output_str));
                if (Err) {
                    std::cout << llvm::toString(std::move(Err)) << std::endl;
                    throw std::runtime_error("Error writing particle typedef.");
                } else {
                    std::cout << "Removed Particle class and replaced with typedef" << std::endl;
                }
            }

            StatementMatcher ParticleCallRecalcNeighbours = traverse(TK_IgnoreUnlessSpelledInSource, cxxMemberCallExpr(
                on(hasType(cxxRecordDecl(hasName("Particle")))),
                callee(cxxMethodDecl(hasName("recalculate_neighbors"))),
                hasArgument(0, expr().bind("ngMaxArg"))
            ).bind("callExpr"));

            void HandleCallRecalculateNeighbours(const MatchFinder::MatchResult& Result, Replacements& Replace) {
                const auto callExpr = Result.Nodes.getNodeAs<CXXMemberCallExpr>("callExpr");
                const Expr *maxArg = Result.Nodes.getNodeAs<Expr>("ngMaxArg");
                const Expr *objectExpr = callExpr->getImplicitObjectArgument();

                std::string objectCodeStr = getSourceText(Result.Context, objectExpr->getSourceRange()).value();
                std::string maxArgStr = getSourceText(Result.Context, maxArg->getSourceRange()).value();

                std::string replacementStr = "wash::recalculate_neighbours(" + objectCodeStr + "," + maxArgStr + ")";

                auto Err = Replace.add(Replacement(
                    *Result.SourceManager, CharSourceRange::getTokenRange(callExpr->getSourceRange()), replacementStr));

                if (Err) {
                    std::cout << llvm::toString(std::move(Err)) << std::endl;
                    throw std::runtime_error("Error trying to rewrite call to recalculate neighbours.");
                } else {
                    std::cout << "Rewrote a call to recalculate neighbours." << std::endl;
                }
            }

            void DeleteParticleCpp() {

            }

        }

    }

}