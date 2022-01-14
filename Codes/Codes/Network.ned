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

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.INetworkNode;
import inet.physicallayer.contract.packetlevel.IRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;
import inet.node.inet.WirelessHost;
import inet.node.inet.StandardHost;
import inet.node.ethernet.Eth100M;

import inet.applications.udpapp.UdpBasicApp;
import inet.applications.contract.IApp;
import inet.physicallayer.contract.packetlevel.IReceiver;

import inet.linklayer.shortcut.ShortcutInterface;
import inet.linklayer.shortcut.ShortcutMac;
import inet.node.inet.AdhocHost;
import inet.node.inet.INetworkNode;

// application 
simple App extends UdpBasicApp
{
    parameters:
        int nbHosts; // BROADCASTCONTROLLER
        @signal[packetSent](type=inet::Packet);
        @signal[packetReceived](type=inet::Packet);
  		@class(Node);

}

module N extends AdhocHost like INetworkNode// WirelessHost
{
    parameters:
        @display("i=block/routing"); // add a default icon 
        @display("i=,gold");
    gates:
        inout gate[];

}

network Dynamique
{
    parameters:
        @display("bgg=100,1,grey95");
        //@figure[title](type=label; pos=0,-1; anchor=sw; color=darkblue);


    //@display("bgb=180,136");
    types:

    submodules:
        visualizer: <default("IntegratedCanvasVisualizer")> like IIntegratedVisualizer if hasVisualizer() {
            @display("p=650,650");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=641,574");
        }


      //  radioMedium: 

        radioMedium: <default("UnitDiskRadioMedium")> like IRadioMedium {
            parameters:
                @display("p=478,636");
        }

        Nodes[75]: N {
            @display("i=device/cellphone;p=413,356");
        }

    connections:

}
