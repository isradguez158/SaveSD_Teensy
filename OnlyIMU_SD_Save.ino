#include "WL_IMU.h"

/* SD Setup */
#include "Scheduler.h" //Taskrunner multitask
#include <SD.h>
#include <SPI.h>

const int chipSelect = BUILTIN_SDCARD;

#define NAME_SD "name4.txt"

//period task
Scheduler scheduler;
void taskSystemLoad(timeUs_t currentTimeUs);
void taskMain(timeUs_t currentTimeUs);
void taskInfo(timeUs_t currentTimeUs);
void taskBlink(timeUs_t currentTimeUs);
task_t tasks[TASK_COUNT] = {
  [TASK_SYSTEM] = DEFINE_TASK("SYSTEM", NULL, taskSystemLoad, TASK_PERIOD_HZ(10), TASK_PRIORITY_MEDIUM_HIGH),
  [TASK_MAIN] = DEFINE_TASK("MAIN", NULL, taskMain, TASK_PERIOD_HZ(1000), TASK_PRIORITY_REALTIME),
  [TASK_INFO] = DEFINE_TASK("INFO", NULL, taskInfo, TASK_PERIOD_MS(2000), TASK_PRIORITY_LOW),
  [TASK_BLINK] = DEFINE_TASK("BLINK", NULL, taskBlink, TASK_PERIOD_HZ(50), TASK_PRIORITY_HIGH),
};

/* SD Setup */


/*Sensors Setup*/
IMU imu;
unsigned long current_time = 0;
unsigned long previous_time = 0;
unsigned long previous_time_ble = 0;
unsigned long Delta_T1 = 20000;
unsigned long Delta_T2 = 20000;
unsigned long t_i, t_pr1, t_pr2;
double t;
double HZ = 1.0 / (Delta_T1 / 1000000.0);
double HZ2 = 1.0 / (Delta_T2 / 1000000.0);

void setup()
{
  delay(3000);
  Serial.begin(115200);
  Serial4.begin(115200);
  delay(100);
  SDCardSetup();
  IMUSetup();

}

void loop()
{
  scheduler.run_scheduler();
  imu.READ();

  t_i = micros();
  t = t_i / 1000000.0;

  if (t_i - t_pr1 > 25000) {
    t_pr1 = t_i;
    Serial.print(imu.LTx);
    Serial.print("  ");
    Serial.println("  "); 


  }

}


void SDCardSetup()
{
  while (!Serial) {
    ;
  }
  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    while (1) {}
  }
  SD.remove(NAME_SD);
  Serial.println("card initialized.");

  delay(5000);

  scheduler.queueClear();
  scheduler.debug(true);//default is false
  scheduler.queueAdd(scheduler.getTask(TASK_SYSTEM));
  scheduler.setTaskEnabled(TASK_INFO, false);
  scheduler.setTaskEnabled(TASK_MAIN, false);
  scheduler.setTaskEnabled(TASK_BLINK, true);
}



void IMUSetup()
{
  imu.INIT();
  delay(500);
  imu.INIT_MEAN();
  current_time = micros();
  previous_time = current_time;
  previous_time_ble = current_time;
}



void taskSystemLoad(timeUs_t currentTimeUs) {
  scheduler.taskSystemLoad(currentTimeUs);
}

void taskInfo(timeUs_t currentTimeUs) {
  scheduler.printTasks();
}

void taskBlink(timeUs_t currentTimeUs) {
  File dataFile = SD.open(NAME_SD, FILE_WRITE);
  if (dataFile) {
    dataFile.print("Time ; ");
    dataFile.print(t_i);
    dataFile.print(" ; LIMU ; ");
    dataFile.print(imu.LTx);
    dataFile.println(" ");
    dataFile.close();
  } else {
    Serial.println("error opening datalog.txt");
  }
}

void taskMain(timeUs_t currentTimeUs) {
}
