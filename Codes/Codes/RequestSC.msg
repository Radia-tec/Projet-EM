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

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

namespace inet;

//cplusplus{{ #include <vector> typedef std::vector<int> IntVector;}}
//class noncobject IntVector;

class RequestSC extends FieldsChunk
{
	int id;// id du noeud 
	int reqTime; // horloge logique de la demande de request;
	int prevClock; //horloge du noeud qui envois/transmet la request
	int k;// kème demande REQUEST
	int prev;
}
