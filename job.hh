/**
 ** \file scheduler/job.hh
 ** \brief Definition of scheduler::Job.
 */

#ifndef SCHEDULER_JOB_HH
# define SCHEDULER_JOB_HH

# include <iosfwd>

# include <libport/symbol.hh>
# include <libport/utime.hh>

# include "object/urbi-exception.hh"
# include "scheduler/coroutine.hh"
# include "scheduler/fwd.hh"
# include "scheduler/tag.hh"

namespace scheduler
{

  enum job_state
  {
    to_start,            ///< Job needs to be started
    running,             ///< Job is waiting for the CPU
    sleeping,            ///< Job is sleeping until a specified deadline
    waiting,             ///< Job is waiting for changes to happen
    joining,             ///< Job is waiting for another job to terminate
    zombie               ///< Job wants to be dead but isn't really yet
  };

  /// A Job represents a thread of control, implemented using coroutines.
  /// The scheduler decides which job to launch and resumes its execution.
  /// The lifetime of a job is the following
  ///
  /// \li   Job()                  : job creation
  ///
  /// \li   start_job()            : add job to the scheduler; the scheduler
  ///                                will call run(), and the job will
  ///                                yield() itself into the scheduler run
  ///                                queue
  ///
  /// \li   work()                 : this method, which must be overridden,
  ///                                does the real work
  ///
  /// \li   terminate()            : called when the job terminates, either
  ///                                because it has indicated it wanted to,
  ///                                because its work() method has
  ///                                returned or because an exception
  ///                                has been thrown from work()
  ///
  /// When the job destructor is called, it will call the
  /// Scheduler::unschedule() routine on its scheduler which will take
  /// care of removing it from all the working queues. The job will
  /// then free the space allocated for its coroutine structure.
  ///
  /// However, due to the way coroutines work, a job may not delete itself
  /// as once the coroutine structure has been freed, it is illegal to
  /// continue its execution, even only to switch to another coroutine.
  /// And if a job switches back to the scheduler before terminating its
  /// destructor, the coroutine structure will not be freed. Because of
  /// that, it is necessary for a job to be deleted by another one or
  /// by the scheduler.
  ///
  /// An additional constraint comes from the fact that even when a job
  /// has terminated its work, its structure may not always be deleted.
  /// For example, other jobs may have kept a reference to this job and
  /// expect to retrieve some information (such as a result) after the
  /// job termination.
  ///
  /// We have several ways of dealing with those constraints and not leaking
  /// memory upon a job termination:
  ///
  /// \li 1. Make sure the job spits out all the useful information concerning
  ///        its state by the way of inter-job communication before asking to
  ///        be deleted.
  ///
  /// \li  2. Make sure the job keeps alive as long as there is at least one
  ///         reference onto it, and that it gets deleted from another
  ///         coroutine.
  ///
  /// We chose to implement the second solution. To achieve this, the
  /// following method has been used:
  ///
  /// \li  1. All the jobs are dynamically allocated. It is forbidden to
  ///         allocate a job from another job stack.
  ///
  /// \li  2. All the references to a job use a \c rJob value, which is a
  ///         reference counted pointer. It is stored as a field in the
  ///         Job structure, hence ensuring that the job destructor will
  ///         not fire as long as we do not override this field.
  ///
  /// \li  3. At creation time, a job creates the \c rJob which will be used
  ///         to represent itself. This \c rJob may be retrieved using
  ///         myself_get() should anyone need to keep a reference to this
  ///         job.
  ///
  /// \li  4. In its terminate() routine, the \c rJob will get rid of its
  ///         self reference. To avoid decreasing the reference count to
  ///         0, it will do so by handing the reference to the scheduler
  ///         using Scheduler::take_job_reference(). This will swap the
  ///         current \c rJob pointer with the scheduler to_kill_
  ///         pointer, which will always be 0 at this stage. As a
  ///         consequence, we ensure that the reference count will at
  ///         least be 1.
  ///
  /// \li  5. After returning from the job, the scheduler will set its
  ///         to_kill_ pointer to 0. As a consequence, if a job has
  ///         called Scheduler::take_job_reference(), the reference count
  ///         will be decremented by 1. If nobody else has a reference on
  ///         the job, its destructor will be called and its memory and
  ///         the one of its associated coroutine structure will be
  ///         freed. If there are other references, the job structure
  ///         will not be destroyed until everyone has dropped all those
  ///         references.

  class Job
  {
  public:
    /// Create a job from another one.
    ///
    /// \param model The parent job. The scheduler and tags will be inherited
    //         from it.
    ///
    /// \param name The name of the new job, or a name derived from \a model
    ///        if none is provided.
    Job (const Job& model, const libport::Symbol& name = SYMBOL ());

    /// Create a new job.
    ///
    /// \param scheduler The scheduler to which this job will be attached.
    ///
    /// \param name The name of the new job, or an automatically created
    ///        one if none is provided.
    explicit Job (Scheduler& scheduler,
		  const libport::Symbol& name = SYMBOL ());

    /// Job destructor.
    ///
    /// The destructor is in charge of unscheduling the job in the
    /// scheduler.
    virtual ~Job ();

    /// Get this job scheduler.
    ///
    /// \return A reference on the job scheduler.
    Scheduler& scheduler_get () const;

    /// Get the underlying coroutine corresponding to this job.
    ///
    /// \return The coroutine structure.
    Coro* coro_get () const;

    /// Has this job terminated?
    ///
    /// \return True if this job is in the \c zombie state.
    bool terminated () const;

    /// Start job by adding it to the scheduler.
    void start_job ();

    /// Run the job. This function is called from the scheduler.
    void run ();

    /// Terminate the job. The job will execute its cleanup method
    /// and inform the scheduler that it is ready to be destroyed.
    void terminate_now ();

    /// Register this Job on its Scheduler so that it is rescheduled
    /// during the next cycle. This should be called from the
    /// currently scheduled job only but must be kept visible to be
    /// callable from the primitives.
    /// \sa yield_until(), yield_until_terminated(),
    /// yield_until_things_changed()
    void yield ();

    /// As yield(), but ask not to be woken up before the deadline.
    /// \sa yield(), yield_until_terminated(), yield_until_things_changed()
    void yield_until (libport::utime_t deadline);

    /// Wait for another job to terminate before resuming execution of
    /// the current one. If the other job has already terminated, the
    /// caller will continue its execution.
    ///
    /// \param other The job to wait for.
    ///
    /// \sa yield(), yield_until(), yield_until_things_changed()
    void yield_until_terminated (Job& other);

    /// Wait for any other task to be scheduled.
    /// \sa yield(), yield_until_terminated(), yield_until()
    void yield_until_things_changed ();

    /// Mark the current job as side-effect free.
    ///
    /// \param s True if the job is now side-effect free.
    ///
    /// Indicate whether the current state of a job may influence other
    /// parts of the system. This is used by the scheduler to choose
    /// whether other jobs need scheduling or not. The default value
    /// for \c side_effect_free is false.
    void side_effect_free_set (bool s);

    /// Is the current job side-effect free?
    ///
    /// \return True if the job is currently side-effect free.
    bool side_effect_free_get () const;

    /// Raise an exception next time this job will be resumed.
    ///
    /// \param e The exception to throw when the job will be scheduled
    ///        again.
    void async_throw (const kernel::exception& e);

    /// Maybe raise a deferred exception. Must be called from the scheduler
    /// while resuming the job execution. For example, BlockedException
    /// may be raised from here if the job has been blocked by a tag.
    void check_for_pending_exception ();

    /// Establish a bi-directional link between two jobs.
    ///
    /// \param other The job to link to.
    ///
    /// \sa unlink()
    void link (Job* other);

    /// Destroy a bi-directional link if it exists.
    ///
    /// \param other The job to unlink from.
    ///
    /// \sa link()
    void unlink (Job* other);

    /// Get the job name
    ///
    /// \return The job name as set from the constructor.
    const libport::Symbol& name_get () const;

    /// Throw an exception if the stack space for this job is near
    /// exhaustion.
    void check_stack_space () const;

    /// Is the job frozen?
    ///
    /// \return This depends from the job tags state.
    bool frozen () const;

    /// Is the job blocked?
    ///
    /// \return This depends from the job tags state.
    bool blocked () const;

    /// Push a tag onto the current job tag stack.
    ///
    /// \param rTag The tag to push.
    void push_tag (rTag);

    /// Pop the latest pushed tag from the job tag stack.
    void pop_tag ();

    /// Copy the tags from another job.
    ///
    /// \param other The other job to copy tags from.
    void copy_tags (const Job& other);

    /// Get the current job state.
    ///
    /// \return The current job state.
    job_state state_get () const;

    /// Set the current job state
    ///
    /// \param state The new job state.
    void state_set (job_state state);

    /// Get this job deadline if it is sleeping.
    ///
    /// \return The date on which this job needs to be awoken.
    ///
    /// This function must not be called unless the job is in the
    /// \c sleeping state.
    libport::utime_t deadline_get () const;

    /// Check if the job can be interrupted.
    ///
    /// \return True if the job cannot be interrupted right now because
    ///         it is executing an atomic operation.
    bool non_interruptible_get () const;

    /// Indicate if the job can be interrupted or not
    ///
    /// \param ni True if the job must not be interrupted until further
    ///        notice.
    void non_interruptible_set (bool ni);

    /// Return a shared pointer on myself. The job must not be
    /// terminated.
    ///
    /// \return A shared pointer on the job.
    scheduler::rJob myself_get () const;

    /// Remember the time we have been frozen since if not remembered
    /// yet.
    ///
    /// \param current_time The current time.
    void notice_frozen (libport::utime_t current_time);

    /// Note that we are not frozen anymore.
    ///
    /// \param current_time The current time.
    void notice_not_frozen (libport::utime_t current_time);

    /// Return the origin since we have been frozen.
    ///
    /// \return 0 if we have not been frozen, the date since we have
    ///         been frozen otherwise.
    libport::utime_t frozen_since_get () const;

    /// Return the current time shift.
    ///
    /// \return The value to subtract from the system time to get the
    ///         unfrozen time of this runner.
    libport::utime_t time_shift_get () const;

    /// Set the current time shift.
    ///
    /// \param ts The new time shift. This should probably only be used
    ///           at runner creation time.
    void time_shift_set (libport::utime_t ts);

  protected:

    /// Must be implemented to do something useful. If an exception is
    /// raised, it will be lost, but before that, it will be propagated
    // into linked jobs.
    virtual void work () = 0;

    /// Will be called if the job is killed prematurely or arrives at
    /// its end. It is neither necessary nor advised to call yield
    /// from this function. Any exception raised here will be lost.
    virtual void terminate ();

  private:
    /// Current job state, to be manipulated only from the job and the
    /// scheduler.
    job_state state_;

    /// Current job deadline. The deadline is only meaningful when the
    /// job state is \c sleeping.
    libport::utime_t deadline_;

    /// The last time we have been frozen (in system time), or 0 if we
    /// are not currently frozen.
    libport::utime_t frozen_since_;

    /// The value we have to deduce from system time because we have
    /// been frozen.
    libport::utime_t time_shift_;

    /// Ensure proper cleanup;
    void terminate_cleanup ();

    /// Scheduler in charge of this job. Do not delete.
    Scheduler* scheduler_;

    /// Myself as long as I have not terminated, 0 otherwise.
    scheduler::rJob myself_;

    /// This job name.
    libport::Symbol name_;

    /// Other jobs to wake up when we terminate.
    jobs_type to_wake_up_;

    /// Coro structure corresponding to this job.
    Coro* coro_;

    /// Tags this job depends on.
    scheduler::tags_type tags_;

    /// List of jobs having a link to this one. If the current job
    /// terminates with an exception, any linked job will throw the
    /// exception as well when they resume.
    jobs_type links_;

    /// Is the current job non-interruptible? If yes, yielding will
    /// do nothing and blocking operations may raise an exception.
    bool non_interruptible_;

    /// Is the current job side-effect free?
    bool side_effect_free_;

    /// The next exception to be propagated if any.
    kernel::exception_ptr pending_exception_;

    /// The exception being propagated if any.
    kernel::exception_ptr current_exception_;
  };

  std::ostream& operator<< (std::ostream&, const Job&);

  /// This exception will be raised to tell the job that it is currently
  /// blocked and must try to unwind tags from its tag stack until it
  /// is either dead or not blocked anymore.
  struct BlockedException : public SchedulerException
  {
    COMPLETE_EXCEPTION (BlockedException);
  };

  // State names to string, for debugging purpose.
  const char* state_name (job_state);

} // namespace scheduler

# include "scheduler/job.hxx"

#endif // !SCHEDULER_JOB_HH
