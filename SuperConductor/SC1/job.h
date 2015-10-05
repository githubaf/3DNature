//--------------------------------------------------------------------
//  Author: CSM SuperConductor team
//  Date: 5/15/02
//
//  Job.h
//
//  class to group and manipulate Work Items.  Work Queue will have
//  a list of jobs. Each job has a priority for scheduling purposes.
//  A job corresponds to a project, or set of frames.
//
//--------------------------------------------------------------------

#ifndef JOB_H
#define JOB_H

#include<qobject.h>
#include<qptrqueue.h>
#include<qstring.h>
#include<qdatetime.h>
#include<qptrlist.h>
#include"workitem.h"

class WorkItem;

typedef QPtrList<WorkItem> Que;

class Job: public QObject
{
   Q_OBJECT

   private:
      uint priority;                 //from 0 to max priority +1 ideally, but no check here
      QString jobName;               //just name of job, for interface
      QString fileName;              //name of the job file
      uint jobID;                    //job identifier should be unique
      Que unassigned;                //undispatched frames(workItems)
      Que working;                   //dispatched frames(WorkItems)
      Que finished;                  //finished frames(WorkItems)
      QDateTime start;               //initializes start time
      QDateTime finish;              //holds start and finish times for job

      bool inList(Que* list, QString itemInfo);
      //returns true if an item in list has the same ItemInfo

   public:
      Job(Job& clone);
      Job(uint priorityValue=1, uint IDValue=0, QString name="NO NAME", QString file="NO FILE");
      ~Job();

      void add(QString itemInfo);
      //adds a WorkItem to the unassigned que
      //uses the given info, the info should be unique within the job

      uint length();                 //returns the # of undispatched frames
      uint totalLength();            //returns the # of dispatched frames and undispatched frames
      WorkItem* getWorkItem();       //returns next udispatched frame, moves frame to working que

      uint getPriority();            //returns priority of this job
      uint getJobID();               //returns the jobID number
	  QString getJobName();          //returns the name of the Job
	  QString getFileName();         //returns the file of the Job

      QTime getStart();              //returns start QDateTime object
      QTime getFinish();             //returns finish QDateTime object
	  
      bool isWorking();              //returns true if jobs are in the working queue
      bool isFinished();             //returns true if no jobs in working queue or unassigned queue
      void setPriority(uint priVal); //changes the priority of the Job
      void setStart();               //sets value of start to current time
      void setFinish();              //sets value of finish to current time

      QTime getEstimatedTime(int &days);
      QTime avgRenderTime(int& days);
      //returns the average time needed for a frame 
      //if more than a day, days will have value greater than one.

   public slots:
      void renderFailed(WorkItem* item, bool& itemInJob);
      //move item to front of unassigned queue and set itemInJob to true

      void renderFinished(WorkItem* item, bool& itemInJob);
      //move item to finished queue and set itemInJob to true

   signals:
      void changed(Job* mutated);     //connected to WorkQueue in WorkQueue::addJob
      void disown(WorkItem* orphan);  //connected to WorkQueue in WorkQueue::addJob
};

#endif
