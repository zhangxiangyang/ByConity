
#include <Functions/FunctionFactory.h>
#include <Functions/FunctionSketch.h>


namespace DB
{

namespace ErrorCodes
{
    extern const int ILLEGAL_COLUMN;
    extern const int BAD_ARGUMENTS;
    extern const int LOGICAL_ERROR;
}


REGISTER_FUNCTION(Sketch)
{
    factory.registerFunction<FunctionQuantilesSketch>("doubleQuantilesSketchEstimate", FunctionFactory::CaseInsensitive);
    factory.registerFunction<FunctionHLLSketch>("doubleHllSketchEstimate", FunctionFactory::CaseInsensitive);
}

}
