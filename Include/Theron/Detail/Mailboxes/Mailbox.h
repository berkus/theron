// Copyright (C) by Ashton Mason. See LICENSE.txt for licensing information.
#ifndef THERON_DETAIL_DIRECTORY_MAILBOXES_MAILBOX_H
#define THERON_DETAIL_DIRECTORY_MAILBOXES_MAILBOX_H


#include <Theron/Address.h>
#include <Theron/Align.h>
#include <Theron/Assert.h>
#include <Theron/BasicTypes.h>
#include <Theron/Defines.h>

#include <Theron/Detail/Messages/IMessage.h>
#include <Theron/Detail/MailboxProcessor/WorkQueue.h>
#include <Theron/Detail/Network/String.h>
#include <Theron/Detail/Threading/SpinLock.h>


#ifdef _MSC_VER
#pragma warning(push)
#pragma warning (disable:4324)  // structure was padded due to __declspec(align())
#endif //_MSC_VER


namespace Theron
{


class Actor;


namespace Detail
{


/**
An individual mailbox with a specific address.
*/
class THERON_PREALIGN(THERON_CACHELINE_ALIGNMENT) Mailbox : public WorkItem
{
public:

    /**
    Default constructor.
    */
    inline Mailbox();

    /**
    \brief Gets the string name of the mailbox.
    The name is arbitrary and identifies the actor within the context of the whole system,
    potentially across multiple connected hosts.
    */
    inline String GetName() const;

    /**
    Sets the name of the mailbox.
    */
    inline void SetName(const String &name);

    /**
    Lock the mailbox, acquiring exclusive access.
    */
    inline void Lock() const;

    /**
    Unlock the mailbox, relinquishing exclusive access.
    */
    inline void Unlock() const;

    /**
    Returns true if the mailbox contains no messages.
    */
    inline bool Empty() const;

    /**
    Pushes a message into the mailbox.
    */
    inline void Push(IMessage *const message);

    /**
    Peeks at the first message in the mailbox.
    The message is inspected without actually being removed from the mailbox.
    \return Pointer to a message, if the mailbox is non-empty, otherwise zero.
    */
    inline IMessage *Front() const;

    /**
    Pops the first message from the mailbox.
    \return Pointer to a message, if the mailbox is non-empty, otherwise zero.
    */
    inline IMessage *Pop();

    /**
    Returns the number of messages currently queued in the mailbox.
    */
    inline uint32_t Count() const;

    /**
    Registers an actor with this mailbox.
    */
    inline void RegisterActor(Actor *const actor);

    /**
    Deregisters the actor registered with this mailbox.
    */
    inline void DeregisterActor();

    /**
    Gets a pointer to the actor registered at this entry, if any.
    \return A pointer to the registered entity, or zero if no entity is registered.
    */
    inline Actor *GetActor() const;

    /**
    Pins the mailbox, preventing the registered actor from being changed.
    */
    inline void Pin();

    /**
    Unpins the mailbox, allowed the registered actor to be changed.
    */
    inline void Unpin();

    /**
    Returns true if the mailbox has been pinned more times than unpinned.
    */
    inline bool IsPinned() const;

private:

    typedef Queue<IMessage> MessageQueue;

    String mName;                               ///< Name of this mailbox.
    mutable SpinLock mSpinLock;                 ///< Thread synchronization object protecting the mailbox.
    MessageQueue mQueue;                        ///< Queue of messages in this mailbox.
    uint32_t mMessageCount;                     ///< Size of the message queue.
    Actor *mActor;                              ///< Pointer to the actor registered with this mailbox, if any.
    uint32_t mPinCount;                         ///< Pinning a mailboxes prevents the actor from being deregistered.

} THERON_POSTALIGN(THERON_CACHELINE_ALIGNMENT);


inline Mailbox::Mailbox() :
  mName(),
  mSpinLock(),
  mQueue(),
  mMessageCount(0),
  mActor(0),
  mPinCount(0)
{
}


THERON_FORCEINLINE String Mailbox::GetName() const
{
    return mName;
}


THERON_FORCEINLINE void Mailbox::SetName(const String &name)
{
    mName = name;
}


THERON_FORCEINLINE void Mailbox::Lock() const
{
    mSpinLock.Lock();
}


THERON_FORCEINLINE void Mailbox::Unlock() const
{
    mSpinLock.Unlock();
}


THERON_FORCEINLINE bool Mailbox::Empty() const
{
    return mQueue.Empty();
}


THERON_FORCEINLINE void Mailbox::Push(IMessage *const message)
{
    mQueue.Push(message);
    ++mMessageCount;
}


THERON_FORCEINLINE IMessage *Mailbox::Front() const
{
    return mQueue.Front();
}


THERON_FORCEINLINE IMessage *Mailbox::Pop()
{
    --mMessageCount;
    return mQueue.Pop();
}


THERON_FORCEINLINE uint32_t Mailbox::Count() const
{
    return mMessageCount;
}



THERON_FORCEINLINE void Mailbox::RegisterActor(Actor *const actor)
{
    // Can't register actors while the mailbox is pinned.
    THERON_ASSERT(mPinCount == 0);
    THERON_ASSERT(mActor == 0);
    THERON_ASSERT(actor);

    mActor = actor;
}


THERON_FORCEINLINE void Mailbox::DeregisterActor()
{
    // Can't deregister actors while the mailbox is pinned.
    THERON_ASSERT(mPinCount == 0);
    THERON_ASSERT(mActor != 0);

    mActor = 0;
}


THERON_FORCEINLINE Actor *Mailbox::GetActor() const
{
    return mActor;
}


THERON_FORCEINLINE void Mailbox::Pin()
{
    ++mPinCount;
}


THERON_FORCEINLINE void Mailbox::Unpin()
{
    THERON_ASSERT(mPinCount > 0);
    --mPinCount;
}


THERON_FORCEINLINE bool Mailbox::IsPinned() const
{
    return (mPinCount > 0);
}


} // namespace Detail
} // namespace Theron


#ifdef _MSC_VER
#pragma warning(pop)
#endif //_MSC_VER


#endif // THERON_DETAIL_DIRECTORY_MAILBOXES_MAILBOX_H

