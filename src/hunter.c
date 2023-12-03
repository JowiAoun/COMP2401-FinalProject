#include "defs.h"

void initHunter(char* name, enum EvidenceType equipment,
                RoomType* room, EvidenceListType* evidenceList, HunterType** hunter){

  *hunter = (HunterType*)allocMemory(sizeof(HunterType));

  strcpy((*hunter)->name, name);
  (*hunter)->equipment = equipment;
  (*hunter)->room = room;
  (*hunter)->evidenceList = evidenceList;
  (*hunter)->fear = 0;
  (*hunter)->boredom = 0;
}

void collectEvidence(HunterType* hunter) {
  if (hunter->room->evidenceList->head != NULL) {
    int ret = removeEvidence(hunter->room->evidenceList, hunter->evidenceList, hunter->equipment);
    if (ret == 1) {
      addEvidence(hunter->evidenceList, hunter->equipment);
      l_hunterCollect(hunter->name, hunter->equipment, hunter->room->name);
    }
  }
}

void moveHunterRooms(HunterType* hunter) {
  // Select a random new room
  int roomIndex = randInt(0, hunter->room->roomList->size);
  RoomNodeType* newRoomNode = hunter->room->roomList->head;
  
  // Traverse to the selected room
  for (int i = 0; i < roomIndex; i++) {
    newRoomNode = newRoomNode->next;
  }

  RoomType* newRoom = newRoomNode->data;

  // Remove hunter from current room
  for (int i = 0; i < NUM_HUNTERS; i++) {
    if (hunter->room->hunters[i] == hunter) {
      hunter->room->hunters[i] = NULL;
      break;
    }
  }

  // Add hunter to new room
  for (int i = 0; i < NUM_HUNTERS; i++) {
    if (newRoom->hunters[i] == NULL) {
      newRoom->hunters[i] = hunter;
      hunter->room = newRoom;
      l_hunterMove(hunter->name, newRoom->name);
      return;
    }
  }
}


void reviewEvidence(HunterType* hunter) {
  if (hunter->evidenceList->size < 3) {
    // Case: not enough evidence to review
    return;
  }

  int numEmf = 0, numTemperature = 0, numFingerprints = 0, numSound = 0;

  EvidenceNodeType* currEvidence = hunter->evidenceList->head;

  for (int i = 0; i < hunter->evidenceList->size; i++) {
    currEvidence = currEvidence->next;
    
    switch (currEvidence->data) {
      case EMF:
        numEmf++;
        break;
      case TEMPERATURE:
        numTemperature++;
        break;
      case FINGERPRINTS:
        numFingerprints++;
        break;
      case SOUND:
        numSound++;
        break;
      default:
        break;
    }
  }

  int evidenceCounts[] = {numEmf, numTemperature, numFingerprints, numSound};
  int count = 0;

  for (int i = 0; i < 4; i++) {
    if (evidenceCounts[i] > 1) {
      count++;
    }
  }

  if (count >= 3) {
    l_hunterReview(hunter->name, LOG_SUFFICIENT);
  } else {
    l_hunterReview(hunter->name, LOG_INSUFFICIENT);
  }
}

void* hunter_thread(void* arg) {
  HunterType* hunter = (HunterType*) arg;

  while (1) {
    usleep(HUNTER_WAIT);

    // 3.1
    if (hunter->room->ghost != NULL) {
      hunter->fear += 1;
      hunter->boredom = 0;
    } else {
      hunter->boredom += 1;
    }
    // 3.2
    int action = randInt(0,2);
    
    switch(action) {
      case 0:
        collectEvidence(hunter);
        break;
      case 1:
        moveHunterRooms(hunter);
        break;
      case 2:
        reviewEvidence(hunter);
        break;
    }

    if (hunter->boredom >= BOREDOM_MAX) {
      removeHunter(hunter);
      l_hunterExit(hunter->name, LOG_BORED);
      return NULL;

    } else if (hunter->fear >= FEAR_MAX) {
      removeHunter(hunter);
      l_hunterExit(hunter->name, LOG_FEAR);
      return NULL;
    }
  }
}