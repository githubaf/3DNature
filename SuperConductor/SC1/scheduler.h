//--------------------------------------------------------------------
//  Author: CSM SuperConductor team
//  Date: 5/15/02
//
//  Scheduler.h
//
//  class to group and manipulate WorkQueues.  Scheduler has
//  a list of WorkQueus, one for each priority.  The higher the priority number,
//  the lower the priority.  Default is a range from 1 (higest) to 9 (lowest), with
//  special priorities set aside.
//
//  Jobs with a priority of zero finish before any other Jobs start.  Jobs with
//  a priority of maxPriority + 1 (10 by default) will not start until all other
//  Jobs have finished.  The priority of a job is easily changed.
//
//--------------------------------------------------------------------

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include"workqueue.h"

typedef QPtrList<WorkQueue> PriList;

class Scheduler: public QObject
{
   Q_OBJECT

   private:
      PriList priorityList;  //List of WorkQueues, one for each possible priority value
      uint maxPriority;      //Maximum Priority Value, lower value => higher priority
      uint currentPriority;  //Range from 1 (highest) to maxPriority (lowest)
   public:
      Scheduler(uint maximumPriority=9);
      //Job of priority 1 would dispatch 9 frames for:
      //every 1 frame a Job of priority 9 would dispatch
      //every 5 frames a Job of priority 5 would dispatch
      //assuming maxPriority is set to 9

      Scheduler(Scheduler& clone);

      ~Scheduler();

      WorkItem* getItem(uint* jobIDAcceptor);
      //retrieve next item(frame) to be rendered and set the jobIDAcceptor to the JobID
      //of the job that the work item comes from.
      //if there are no more items, null is returned and jobIDAcceptor is not set

      void renderFailed(WorkItem* item);
      //make work item available to dispatch to another resource

      void renderFinished(WorkItem* item);
      //record change of status

      bool addJob(Job* job);
      //if job is valid add job to scheduler
      //job priority should be within range (0 < job priority < maxPriority+1)
      //job ID should be unique within this Scheduler object
      //returns true if the job was added

      bool endJob(uint jobID);
      //if job in the scheduler remove it

      void setPriority(uint jobID, uint priorityValue);
      //move the job to a different priority if the priority is in range

      QString getJobName(uint jobID);

      uint getMaxPriority();
      uint getCurrentPriority();

   public slots:
      void update(Job* mutated); //pass on signal that job changed.
   signals:
      void changed(Job* mutated);
};

#endif
