#pragma Once

#include <stdint.h>

class uRosComms {
    public:
        uRosComms();
        void Init();
        void Receive();
        void Publish(const uint8_t sensor_status);
    private:
};
