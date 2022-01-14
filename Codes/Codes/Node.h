//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef NODE_H_
#define NODE_H_


#include <string.h>
#include <omnetpp.h>
#include <vector>
#include <iostream>
#include <bits/stdc++.h>
#include <stdio.h>
#include <time.h>

using namespace omnetpp;

#include "inet/applications/udpapp/UdpBasicApp.h"
#include "inet/common/packet/Packet.h"
#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"

#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/InterfaceTable.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/common/packet/chunk/Chunk.h"
#include "inet/common/packet/chunk/SliceChunk.h"
#include "inet/common/packet/ReassemblyBuffer.h"
#include "inet/common/socket/SocketMap.h"


#include "Token_m.h"
#include "RequestSC_m.h"

using namespace inet;
using namespace std;

static int id_counter=0;

int cptFini=0;


int moyenneSC;

int nbrMessage =0;

class Node : public inet::UdpBasicApp
{
    public:
        Node();
        virtual ~Node();
        /*                              Module functions                                    */
        virtual void initialize(int stage) override;
        virtual void handleMessageWhenUp(cMessage *msg) override;



        //virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override
        virtual bool handleOperationStage(LifecycleOperation *operation, IDoneCallback *doneCallback) override
        { Enter_Method_Silent(); throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName()); return true; }
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }; // permet de dÃ©finir le nombre d'Ã©tapes d'initialisation (sans Ã§a il n'y en a que 2)

        // ETATS D'UN NOEUD
        #define REQUESTING 0
        #define NOT_REQUESTING 1
        #define CRITICAL_SECTION 2

        // TAG DES MSG
        #define TOKEN 3
        #define REQUEST 4

        //TAG utilisé dans la fonction HandleMessagWhenUp
        #define FinSC 5

        // VARIABLES

        int token; //variable qui représente la présence du jeton
        int state;


        int *ln;
        int *rn;

        clock_t t1, t2;
        float temps;

        int local_clock=0;
        int compteurSC =0;
        int fini=0;// boolean pour finir au bout de trois SC
        int k=0;//compteur nombre de demande du jeton
        int nbVoisins=0;

        typedef struct data{int id; int clock;}data;
        vector <data> requestL;
        vector <data> tokenL;

        vector <int> qL;
        int contains(vector <int> v, int val);
        int containsL(vector <data> v,int id, int clock);

        int id=id_counter++;
        int wlanInterfaceId; // enregistre au dÃ©but pour plus de performances
        map <int, ReassemblyBuffer*> ReassembleBuffers;
        int nbHosts;
        int seq=0;

        int max(int a ,int b);
        void receiveMessage( const IntrusivePtr<const FieldsChunk>& msg, int TAG);
        void sendMessage(int TAG, int dest);
        void releaseSC();
        void enterSC();

        simtime_t getRandTime(int interval);

};

    Define_Module(Node);
#endif /* NODE_H_ */

