# smart-home-energy-management-system
Final Year Dissertation Project - IoT Smart Home Energy Management System using Arduino UNO R4 WiFi, Blynk Cloud, DS3231 RTC, PIR Motion Sensor and Energy Monitoring.



## Final Year Dissertation Project

### Author
**Jarine Anika**  
BSc Computer Science  
Solent University  
Academic Year 2025–2026

---

## Project Overview

The Smart Home Energy Management System is an Internet of Things (IoT) based solution developed to improve household energy efficiency through automation, monitoring and remote control. The system integrates Arduino UNO R4 WiFi, Blynk IoT Cloud, DS3231 Real-Time Clock (RTC), PIR motion sensing and energy monitoring functionality to provide intelligent control of home appliances.

The project aims to reduce unnecessary energy consumption by automatically controlling appliances based on user-defined schedules, occupancy detection and predefined energy-saving rules.

---

## Project Objectives

- Develop an IoT-based smart home prototype.
- Enable remote control of household appliances through a mobile application.
- Implement PIR-based motion detection.
- Automate appliance operation using RTC scheduling.
- Monitor appliance energy consumption in real time.
- Generate energy-saving statistics and reports.
- Provide user notifications and alerts.
- Evaluate system performance through functional testing and user feedback.

---

## Key Features

### Remote Appliance Control
Users can remotely control household appliances through the Blynk mobile application.

### Motion Detection
A PIR sensor detects human movement and updates the system status in real time.

### RTC-Based Scheduling
The DS3231 RTC module enables automatic appliance operation based on user-defined schedules.

### Energy Monitoring
The system continuously monitors appliance usage and calculates energy consumption.

### Notification System
Users receive mobile notifications for:
- Motion detection events
- Scheduled reminders
- Extended appliance operation
- Automatic shutdown events

### Automatic Energy Saving
The system automatically switches off appliances after predefined operating durations to minimise energy waste.

### Live Dashboard
Real-time system data is displayed through the Blynk IoT dashboard.

---

## Hardware Components

| Component | Purpose |
|------------|------------|
| Arduino UNO R4 WiFi | Main microcontroller |
| DS3231 RTC Module | Time scheduling |
| PIR Motion Sensor | Motion detection |
| Servo Motor | Smart door control |
| Relay Module | Appliance switching |
| LED Light | Lighting simulation |
| Fan Module | Fan simulation |
| WiFi Connectivity | IoT communication |

---

## Software Technologies

- Arduino IDE
- C/C++
- Blynk IoT Platform
- GitHub
- WiFiS3 Library
- Servo Library
- RTClib

---

## System Architecture

The system consists of:

1. Arduino UNO R4 WiFi as the processing unit.
2. PIR sensor for occupancy detection.
3. DS3231 RTC module for scheduling.
4. Relay-controlled appliances.
5. Blynk Cloud platform for communication.
6. Mobile dashboard for monitoring and control.

---

## Repository Structure

```text
smart-home-energy-management-system
│
├── Source_Code
│   └── SmartHome.ino
│
├── Dissertation_Report
│   └── Final_Report.pdf
│
├── diagram
│   ├── Architecture.jpg
│   ├── Usecase.jpg
│   └── SequenceA.jpg
│
├── Screenshots
│
├── Testing
│
├── Presentation
│
└── README.md
```

---

## Testing and Evaluation

The system was evaluated through:

- Functional testing
- Hardware integration testing
- User acceptance testing
- Energy monitoring validation
- Notification verification

All major project requirements were successfully implemented and validated.

---

## Results

The developed prototype successfully demonstrated:

- Real-time appliance control
- Motion detection functionality
- Automated scheduling
- Energy monitoring
- Mobile notifications
- Energy-saving automation

User evaluation results indicated positive feedback regarding usability, convenience and energy awareness.

---

## Future Improvements

Potential future enhancements include:

- Machine learning-based energy prediction
- Cloud database integration
- Smart energy analytics dashboard
- Voice assistant integration
- Renewable energy monitoring
- Multi-room smart home management

---

## Academic Context

This repository was created to support the implementation, testing and documentation of the final year dissertation project submitted to Solent University for the BSc Computer Science degree programme.

---

## License

This project is intended for academic and educational purposes.
