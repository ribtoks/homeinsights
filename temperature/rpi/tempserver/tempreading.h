#ifndef TEMPREADING_H
#define TEMPREADING_H

struct TempReading {
    TempReading(): m_Temperature(0.0f), m_SensorID(-1) {}
    
    double m_Temperature;
    int m_SensorID;
};

#endif // TEMPREADING_H
