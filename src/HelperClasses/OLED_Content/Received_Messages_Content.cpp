#include "Received_Messages_Content.h"

Received_Messages_Content::Received_Messages_Content(bool showReadMsgs, bool wrapAround)
    : showReadMsgs(showReadMsgs), wrapAround(wrapAround)
{
    type = ContentType::STATUS;
}

Received_Messages_Content::~Received_Messages_Content()
{
}

void Received_Messages_Content::encDown()
{
    bool iteratorAtEnd = false;
    if (showReadMsgs) 
    {
        iteratorAtEnd = LoraUtils::IsMessageIteratorAtEnd();
    }
    else
    {
        iteratorAtEnd = LoraUtils::IsUnreadMessageIteratorAtEnd();
    }

    if (!wrapAround && iteratorAtEnd)
    {
        return;
    }

    

    if (showReadMsgs) 
    {
        LoraUtils::IncrementMessageIterator();
    }
    else
    {
        LoraUtils::IncrementUnreadMessageIterator();
    }

    printContent();
}

void Received_Messages_Content::encUp()
{
    bool iteratorAtStart = false;
    if (showReadMsgs) 
    {
        iteratorAtStart = LoraUtils::IsMessageIteratorAtBeginning();
    }
    else
    {
        iteratorAtStart = LoraUtils::IsUnreadMessageIteratorAtBeginning();
    }

    if (!wrapAround && iteratorAtStart)
    {
        return;
    }

    if (showReadMsgs) 
    {
        LoraUtils::DecrementMessageIterator();
    }
    else
    {
        LoraUtils::DecrementUnreadMessageIterator();
    }

    printContent();
}

void Received_Messages_Content::printContent()
{
    Display_Utils::clearContentArea();

#if DEBUG == 1
    Serial.println("Received_Messages_Content::printContent()");
#endif

    size_t numMsgs;

    if (showReadMsgs)
    {
        numMsgs = LoraUtils::GetNumMessages();
    }
    else
    {
        numMsgs = LoraUtils::GetNumUnreadMessages();
    }

    if (numMsgs == 0)
    {
        display->clearDisplay();

        display->setCursor(20, 12);
        display->print("No messages");
        display->display();
        return;
    }

    // uint64_t userLookup = msgSenderUserIDs[msgIdx];

    MessageBase *msg;

    if (showReadMsgs)
    {
        msg = LoraUtils::GetCurrentMessage();
    }
    else
    {
        msg = LoraUtils::GetCurrentUnreadMessage();
    }

    std::vector<MessagePrintInformation> msgPrintInfo;
    msg->GetPrintableInformation(msgPrintInfo);

    // Start in content area
    size_t lineNumber = 2;
    for (auto &mpi : msgPrintInfo)
    {
        display->setCursor(Display_Utils::alignTextLeft(), Display_Utils::selectTextLine(lineNumber));
        display->print(mpi.txt);
        lineNumber++;

        if (lineNumber > 3)
        {
            break;
        }
    }

    printMessageAge(NavigationUtils::GetTimeDifference(msg->time, msg->date));

    display->display();

    delete msg;
}

void Received_Messages_Content::printMessageAge(uint64_t timeDiff)
{
    char buffer[8];

    if (timeDiff > 0xFFFFFFFF)
    {
        // display->print(">1d");
        sprintf(buffer, ">1d");
    }
    else if ((timeDiff & 0xFF000000) >> 24)
    {
        uint8_t hours = (timeDiff & 0xFF000000) >> 24;
        // display->print(hours);
        // display->print(F("h"));
        sprintf(buffer, "%dh", hours);
    }
    else if ((timeDiff && 0xFF0000) >> 16)
    {
        uint8_t minutes = (timeDiff && 0xFF0000) >> 16;
        // display->print(minutes);
        // display->print(F("m"));
        sprintf(buffer, "%dm", minutes);
    }
    else
    {
        // display->print(F("<1m"));
        sprintf(buffer, "<1m");
    }

    display->setCursor(Display_Utils::alignTextRight(strlen(buffer)), Display_Utils::selectTextLine(2));

    display->print(buffer);
}

// void Received_Messages_Content::getMessages()
// {
//     msgSenderUserIDs.clear();
//     DynamicJsonDocument *doc;
//     uint64_t currentUserIDPointedTo = 0;

//     if (msgIdx < msgSenderUserIDs.size())
//     {
//         currentUserIDPointedTo = msgSenderUserIDs[msgIdx];
//     }

//     if (showReadMsgs)
//     {
//         if (lastRefreshTime < Network_Manager::getLastMessageInsertDeleteTime())
//         {
//             lastRefreshTime = Network_Manager::getLastMessageInsertDeleteTime();
//             doc = Network_Manager::getMessages();
//         }
//         else
//         {
//             return;
//         }
//     }
//     else
//     {
//         if (lastRefreshTime < Network_Manager::getLastUnreadMessageInsertDeleteTime())
//         {
//             lastRefreshTime = Network_Manager::getLastUnreadMessageInsertDeleteTime();
//             doc = Network_Manager::getMessagesUnreadMessages();
//         }
//         else
//         {
//             return;
//         }
//     }

//     for (uint16_t i = 0; i < (*doc)["UserIDs"].size(); i++)
//     {
//         msgSenderUserIDs.push_back((*doc)[i]);
//     }

//     auto it = std::find(msgSenderUserIDs.begin(), msgSenderUserIDs.end(), currentUserIDPointedTo);
//     if (it != msgSenderUserIDs.end())
//     {
//         msgIdx = std::distance(msgSenderUserIDs.begin(), it);
//     }
//     else
//     {
//         msgIdx = 0;
//     }

//     delete doc;
// }
