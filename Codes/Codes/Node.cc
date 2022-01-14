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

#include "Node.h"



Node::Node() {
   // Node 0 auras le TOKEN a l'initialisation du réseau
    if (id != 0)
        token=0;
    else
        token=1;

    state=NOT_REQUESTING;
}

Node::~Node()
{}

void Node::initialize(int stage)
{
    UdpBasicApp::initialize(stage);

    if(stage == INITSTAGE_APPLICATION_LAYER)
    {
        nbHosts         = par("nbHosts");

        //initialisation de LN et RN

        ln=(int*)malloc(nbHosts*sizeof(int));
        rn=(int*)malloc(nbHosts*sizeof(int));

        for(int i=0;i<nbHosts;i++)
        {
            ln[i]=0;
            rn[i]=0;
        }

    }

    if(stage == INITSTAGE_LAST)
    {
        InterfaceTable* t = dynamic_cast<InterfaceTable*> (getModuleByPath(par("interfaceTableModule")));
        wlanInterfaceId = t->findInterfaceByName("wlan0")->getInterfaceId();
    }

}

void Node::handleMessageWhenUp(cMessage *msg)
{
    if (msg==selfMsg ) {
        ASSERT(msg == selfMsg);
        switch (selfMsg->getKind()) {
            case START:
                processStart();

                if(id%2 ==0 && id <40 ) // tous les noeuds avec un id pair et inferieur a 40 vont initier une demande
                {
                selfMsg->setKind(SEND);

                cout << "ID "<< id << "et je vais initier  un send."<<endl;
                scheduleAt(simTime()+ par("sendInterval") +2, selfMsg);
                }

                break;
            case SEND:
               sendMessage(REQUEST,0 );
                break;
            case STOP:
                processStop();
                break;
            case FinSC :
                state= NOT_REQUESTING;
                cout<< " ID "<<id<<",je sors de de la SC"<< endl;
                releaseSC();
                break;
            default:
                throw cRuntimeError("Invalid kind %d in self message", (int)selfMsg->getKind());
        }
    }
    else
    {
        Packet* p =dynamic_cast<Packet*> (msg);
        while(true)
        {
            if(p->getTotalLength() == p->getFrontOffset())
                break;
            const inet::IntrusivePtr<const FieldsChunk> m = dynamic_pointer_cast<const FieldsChunk> (p->popAtFront());

            // RECEPTION MESSAGES -> RequestSC ou TOKEN
            if(dynamic_pointer_cast<const RequestSC> (m)!= nullptr)
            {
                receiveMessage(m, REQUEST); // Reception REQUEST
            }
            else if (dynamic_pointer_cast<const Token> (m)!= nullptr)
            {
                receiveMessage(m, TOKEN); // Reception TOKEN
            }
            else
                cout<<"not handled message"<<m << endl;

        }
        p->setBackOffset(p->getFrontOffset());
        delete p;

    }
}

//Cette fonction calcule le max entre deux entier
int Node::max(int a , int b)
{
    if(a<b)
        return b;
    else
        return a;
}

// Pour cette fonction, le prametre DEST ne sert a rien si le TAG == REQUEST
// il est utilisé pour l'envois du jeton, le destinataire du jeton
void Node::sendMessage(int TAG,int dest)
{
    local_clock++;

    if(TAG == REQUEST){
        if(token ==0 )
        {
            t1=clock();
            k++;//incremente le nombre de demande d'entree en SC

            state=REQUESTING;

            cout <<"SEND REQUEST ID" << id <<endl;

            rn[id]+=1;

            Packet *p=new Packet();
            const auto& m = makeShared<RequestSC>();
            m->setChunkLength(B(par("messageLength")));

            m->setId(id);
            m->setK(k);
            m->setReqTime(local_clock);
            m->setPrevClock(local_clock);
            p->insertAtFront(m);
            m->setPrev(id);

            p->addTag<InterfaceReq>()->setInterfaceId( wlanInterfaceId );
            socket.sendTo(p,Ipv4Address::ALLONES_ADDRESS,destPort);
        }
    }
    else if(TAG == TOKEN ){

        cout <<" SEND TOKEN ID = " << id<<endl;

        Packet *p1=new Packet();
        const auto& m = makeShared<Token>();

        //Configuration du message m TOKEN a envoyer
        m->setChunkLength(B(par("messageLength")));
        m->setDest(dest);
        m->setPrev(id);
        m->setPrevClock(local_clock);
        m->setSendTime(local_clock);

        int size = qL.size();

        m->setQArraySize(size);
        m->setLnArraySize(nbHosts);

        for(int i=0; i<nbHosts; i++)
            m->setLn(i, ln[i]);

        int x;


        for(int i=0; i< qL.size(); i++)
        {
            x= qL.at(i);
            m->setQ(i, x);
            cout<<" je vien d'inserer "<<m->getQ(i);
        }

        p1->insertAtFront(m);
        p1->addTag<InterfaceReq>()->setInterfaceId( wlanInterfaceId );

        // Il n'est plus propriétaire du jeton
        token=0;

        socket.sendTo(p1,Ipv4Address::ALLONES_ADDRESS,destPort);

        //vider la liste Q
        qL.clear();

        //Changement d'etat
        state=NOT_REQUESTING;
    }

    // Pour programmer une nouvelle demande de jeton
    if (token == 0 && state != CRITICAL_SECTION && state != REQUESTING && !fini && id%2==0 && id <40){
        simtime_t d = simTime()+ par("sendInterval");

        if (stopTime < SIMTIME_ZERO || d < stopTime) {
            cancelEvent(selfMsg);
            selfMsg->setKind(SEND);
            //cout << " Je Relance une demande dans  = "<< d + 10<< "ID = "<< id<< endl;
            scheduleAt(d+10, selfMsg); // schedule timer pour prochain envoie de message
        }
        else {
            selfMsg->setKind(STOP);
            scheduleAt(stopTime, selfMsg);
        }
    }
}

//vérifier si la liste contient une requete (Request/Token) déja vu
int Node::containsL(vector <data> v,int id, int clock){

    for(int i=0 ; i< v.size(); i++)
    {
        if (v[i].id== id && v[i].clock == clock )
            return 1;
    }
    return 0;
}


void Node::receiveMessage( const IntrusivePtr<const FieldsChunk>& msg, int TAG)
{
    // Demande d'entrer en Section critique
    if (TAG == REQUEST ){

        const inet::IntrusivePtr<const RequestSC> m = dynamic_pointer_cast<const RequestSC> (msg);

        //Recupère l'identifiant de l'expéditeur de la request
        int source=m->getId();
        int reqTime=m->getReqTime();

        //Si le noeud n'a jamais recu cette requete alors il faut la traiter et la transmettre si token =0
        if(!containsL(requestL,source, reqTime) && source !=id){
            nbrMessage++;
            cout<<" NODE = "<<id<<"  viens de recevoir une nouvelle REQUEST "<<m->getId()<<" de la part de "<< m->getPrev()<<endl;

            int young = 1;

            for(int i=0; i<requestL.size(); i++)
            {
                //Si il existe une requete du meme noeud plus ancienne on supprime l'ancienne
                if (requestL[i].id == source && requestL[i].clock < reqTime)
                {
                   // cout<<"je supprime une ancienne REQUEST "<< source<<endl;
                    requestL.erase(requestL.begin()+i);
                }

                if(requestL[i].id == source && requestL[i].clock > reqTime)
                {
                    int young = 0;
                   // cout<<" j'ai trouve une requete plus jeune"<<source<<endl;
                }

            }


            if (young)
            {
                cout<<"NODE "<<id<<" ajoute la REQUEST ("<<source<< " - "<<reqTime<<") a RequestL"<<endl;
                data newR;
                newR.id= source;
                newR.clock=reqTime;
                requestL.emplace_back(newR);
            }


            local_clock=max(local_clock, m->getPrevClock())+1;
            int kRec =m->getK();

            rn[source]=max(rn[source],kRec);

            // Si le noeud courant n'a pas besoin du TOKEN
            if((token == 1) && ( state== NOT_REQUESTING) && (rn[source] > ln[source]) )
            {
                sendMessage(TOKEN, source);
            }
            else if(token ==0)//Le noeud n'a pas le jeton, il retranssmet la demande
            {
                           cout <<" NODE "<< id <<" Je retransmet la request ( initiateur= "<< source<<" sendTime= "<< reqTime<<" )" << endl;
                           Packet *p=new Packet();
                           const auto& m = makeShared<RequestSC>();
                           m->setChunkLength(B(par("messageLength")));

                           m->setId(source);
                           m->setK(kRec);
                           m->setReqTime(reqTime);
                           m->setPrevClock(local_clock);
                           m->setPrev(id);

                           p->insertAtFront(m);
                           p->addTag<InterfaceReq>()->setInterfaceId( wlanInterfaceId );
                           socket.sendTo(p,Ipv4Address::ALLONES_ADDRESS,destPort);
            }

            if(state == CRITICAL_SECTION)//Si le NODE est en SC, il ajoute l'identifiant du demandeur a la Q
            {
                int cont= contains(qL,source); // vérifier l'existence du node dans la Q

                    if( !cont &&  rn[source]>ln[source])
                    {
                        qL.push_back(source);// ajouter l'identifiant du node a la Q
                    }
            }
        }
    }else
    {
        if (TAG == TOKEN)// RECEPTION JETON
        {

            const inet::IntrusivePtr<const Token> m = dynamic_pointer_cast<const Token> (msg);

            int dest = m->getDest();
            int h = m->getSendTime();
            nbrMessage++;
            if(dest == id && state != CRITICAL_SECTION){ // Je suis l'initiateur et je recois le token -> SC

                cout << "ID = "<< id <<" RECEIVE TOKEN  ("<<dest<<" , "<<m->getPrev()<<")--- > entree en section critique" <<endl;

                local_clock=max(local_clock, m->getPrevClock())+1;

            //Recuperer la liste LN
            for(int i=0; i<nbHosts;i++)
                ln[i]=m->getLn(i);

            // Recuperer la liste Q
            int sizeQ=m->getQArraySize();
            for(int i=0; i<sizeQ; i++)
            {
                int x= m->getQ(i);
                   qL.push_back(x);
            }

            state  = CRITICAL_SECTION;
            token = 1;

            enterSC();// Aller en Section critique

            }
            else if (!containsL(tokenL,dest,h) && id !=dest)//si je ne suis pas le demandeur et que je n'ai jamais vu cette requete, je la retranssmet
            {
                //supprimé les ancienne requete token
                for(int i=0; i<tokenL.size(); i++)
                {
                    if (tokenL[i].id == dest && tokenL[i].clock < h)
                        tokenL.erase(tokenL.begin()+i);
                }

                data newR;
                newR.id= dest;
                newR.clock= h;
                tokenL.emplace_back(newR);

                local_clock=max(local_clock, m->getPrevClock())+1;
                int sizeQ=m->getQArraySize();

                cout << "RECEIVE TOKEN dest = "<<dest<<" h = "<<h<<" ID = "<< id << " from "<<m->getPrev()<<" je le retranssmet" <<endl;

                Packet *p1=new Packet();
                const auto& m1 = makeShared<Token>();

                //Configuration du message m a envoyer
                m1->setChunkLength(B(par("messageLength")));
                m1->setSource(m->getSource());

                m1->setSendTime(h);
                m1->setDest(dest);
                m1->setPrev(id);
                m1->setPrevClock(local_clock);

                m1->setQArraySize(sizeQ);
                m1->setLnArraySize(nbHosts);

                int x;
                for(int i=0; i<nbHosts; i++)
                     m1->setLn(i, m->getLn(i));

                 for(int i=0; i< sizeQ; i++)
                 {
                     x= m->getQ(i);
                     m1->setQ(i, x);
                 }

                 p1->insertAtFront(m1);
                 p1->addTag<InterfaceReq>()->setInterfaceId( wlanInterfaceId );

                 socket.sendTo(p1,Ipv4Address::ALLONES_ADDRESS,destPort);

            }
        }
    }


}


//Fonction qui renvois true si le vector contient la valeur passé en paramètres, false sinon
int Node::contains(vector <int> v, int val){

    for(int i=0 ; i< v.size(); i++)
    {
        if (v[i]== val)
            return 1;
    }
    return 0;
}

/// Fonction executer a choque sortie de section critique
void Node::releaseSC(){

    ln[id]=rn[id];
    int isVide=0;
    compteurSC++;

    if(compteurSC == 2)
    {

        fini=1;
        cptFini++;
        cout << " j'ai fini mes 3 SC CPTFINI ="<<cptFini<< endl;
        t2=clock();
        temps = (float)(t2-t1)/CLOCKS_PER_SEC;
        cout<<" temps = "<<temps<<endl;
        moyenneSC+=temps;


    }
    else
        cout << " je viens de finir ma "<<compteurSC<< " SC"<<endl;

    // Pour tt les noeuds sauf noeud courant (qui n'est pas dans la q) et que rn[site]>ln[site]
    // ajouter site a Q - VOIR ALGO COURS


        if(! qL.empty())
        {
            token=0;
            //extraire premiere element de la Q et stocker sa valeur
            int& i = qL.front();
            int x=i;
            //supprimer le premier element
            qL.erase(qL.begin());

            // envoyer le TOKEN
            sendMessage(TOKEN, x);

        }
        if(cptFini==20)
        {

            cout<<"TOUS LE MONDE A FINI SA SC"
            moyenneSC=moyenneSC/50;
            cout << "moyenne = "<< moyenneSC<<endl;
            cout<<"nbr message = "<< nbrMessage <<endl;
        }

}

//je schedule le moment ou le state se mettera a NOT_REQUESTING
void Node::enterSC(){

  simtime_t d = simTime()+ par("sendInterval");
    if (stopTime < SIMTIME_ZERO || d < stopTime) {
        cancelEvent(selfMsg);
        selfMsg->setKind(FinSC);//voir la fonction HandlemessageWhenUp

        cout << "Je rentre en SC ID = "<< id<<endl;
        scheduleAt(d+3, selfMsg); // schedule timer pour prochaine envoie de message
    }
    else {
        selfMsg->setKind(STOP);
        scheduleAt(stopTime, selfMsg);
    }
}

simtime_t Node::getRandTime(int interval)
{

    return *(new SimTime(rand() %interval, SIMTIME_MS)) ;
}



