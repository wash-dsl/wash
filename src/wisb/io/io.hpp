
namespace wash {

namespace io {
    
    enum class OutputType { 
#ifdef WASH_HDF5_SUPPORT
        HDF5, 
#endif
        ASCII 
    };

    struct IOOptions {
        int iterationSummaryFrequency = 1;
        int fileDumpFrequency = 1;
        bool timeEventEnabled = true;
    };

    template<OutputType type, int dim>
    class FileIO {
    
    public:
        int handle_iteration();

        int handle_timing_event();
    };
}

}