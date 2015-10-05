//--------------------------------------------------------------------
//  Author: CSM SuperConductor team
//  Date: 5/15/02
//
//  WorkQueue.h
//
//  class to group and manipulate Jobs.  Work Queue has
//  a list of jobs. Each job has a priority for scheduling purposes.
//  A job corresponds to a project, or set of frames.
//
//  The Work Queue current pointer keeps track of the next Job to be
//  processed.  It should not be changed anywhere except in the nextItem
//  function.
//
//--------------------------------------------------------------------

#ifndef WORKQUEUE_H
#define WORKQUEUE_H

#include"job.h"

class WorkQueue:public QObject
{
   Q_OBJECT

   private:
      uint priority;      //priority of this work queue and all jobs in it
      uint frameCount;    //keeps track of the number of work items a job has dispatched
      uint maxFrames;     //maximum number of work items a job can dispatch before another job gets a chance

      QPtrList<Job> jobListing;       //keeps track of all of the jobs in this queue
      QPtrList<WorkItem> workHouse;   //List to hold WorkItems whose jobs were closed while they
                                      //were in the working queue
      int at(uint jobID);
      //if there is a job in this list with a matching jobID returns the index of the job
      //otherwise returns negative one.

   public:
      WorkQueue(uint maximumFrames=10, uint currentPriority=1);
      //sets maxFrames to window, default is zero

      WorkQueue(WorkQueue& clone);
      //makes a deep copy of the clone

      ~WorkQueue();

      WorkItem* nextItem(uint currentPriority,uint* jobIDAcceptor);
      //returns next work item scheduled to be dispatched or null
      //if priority does not match then returns null

      bool addJob(Job* job);
      //adds a job to the WorkQueue, and returns true if successful

      bool removeJob(uint jobID);
      //removes a job from the WorkQueue and deletes it

      Job* getJob(uint jobID);
      //removes a job from the WorkQueue without deleting it

      bool renderFailed(WorkItem* item);
      //if the WorkItem is in this Queue, the failure is handled
      //and true is returned.

      bool renderFinished(WorkItem* item);
      //if the WorkItem is in this Queue, the frame is finished
      // and true is returned.

      bool containsJobID(uint jobID);
      //if there is a job in this list with a matching jobID return true
      //else return false

      void setMaxFrames(uint window);
      uint getMaxFrames();
      uint getPriority();

   public slots:
      void store(WorkItem* homeless);
      //places a homeless workitem in the workHouse.  These workitems come from deleting unfinished Jobs

      void update(Job* mutated);
      //passes signal onto the scheduler that a job has changed

   signals:
      void changed(Job* mutated);  //connected to the Scheduler in Scheduler::Constructors
};

#endif
