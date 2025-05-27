#pragma Once

class uRosComms {
    public:
        uRosComms();
        void Init();
        void Receive();
        void Publish(const bool sensor_status);
    private:
        static float linear_vel;
        static float angular_vel;
};
