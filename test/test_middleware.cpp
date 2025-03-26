#include "unit_test_framework.h"

#include "etl/middleware.h"

using namespace middleware;

namespace
{
  SUITE(test_middleware)
  {
    //*************************************************************************
    TEST(test_call)
    {
        using GetTemperature = Method<int>;
        using SetTemperature = Method<int>;

        using TemperatureInterface = Interface<GetTemperature, SetTemperature>;

        using TIS = Skeleton<TemperatureInterface>;
        using PIS = Proxy<TemperatureInterface>;

        PIS pis;

        //pis.call<GetTemperature>(3);

        int result{};
        //pis.call<SetTemperature>(result);

        CHECK_EQUAL(result, 3);
    }
  }
}
