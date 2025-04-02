#include "unit_test_framework.h"

#include "etl/middleware.h"

using namespace middleware;

namespace
{
  SUITE(test_middleware)
  {
    using GetTemperature = Method<int>;
    using SetTemperature = Method<void, int>;
    using TemperatureInterface = Interface<GetTemperature, SetTemperature>;
    
    class TIS: public Skeleton<TemperatureInterface>
    {
    
    };

    using PIS = Proxy<TemperatureInterface>;

    //*************************************************************************
    TEST(test_call)
    {
        PIS pis;
        //TIS tis;

        pis.call<SetTemperature>(3);
        future = pis.call<GetTemperature>();

        int result{};

        CHECK_EQUAL(result, 3);
    }
  }
}
