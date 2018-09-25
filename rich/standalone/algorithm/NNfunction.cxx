#include "NNfunction.h"
#include <cmath>

double NNfunction::Value(int index,double in0,double in1,double in2,double in3,double in4,double in5) {
   input0 = (in0 - 0)/1;
   input1 = (in1 - 0)/1;
   input2 = (in2 - 0)/1;
   input3 = (in3 - 0)/1;
   input4 = (in4 - 0)/1;
   input5 = (in5 - 0)/1;
   switch(index) {
     case 0:
         return neuron0x860e698();
     default:
         return 0.;
   }
}

double NNfunction::Value(int index, double* input) {
   input0 = (input[0] - 0)/1;
   input1 = (input[1] - 0)/1;
   input2 = (input[2] - 0)/1;
   input3 = (input[3] - 0)/1;
   input4 = (input[4] - 0)/1;
   input5 = (input[5] - 0)/1;
   switch(index) {
     case 0:
         return neuron0x860e698();
     default:
         return 0.;
   }
}

double NNfunction::neuron0x85fe910() {
   return input0;
}

double NNfunction::neuron0x85feae8() {
   return input1;
}

double NNfunction::neuron0x860cfa8() {
   return input2;
}

double NNfunction::neuron0x860d180() {
   return input3;
}

double NNfunction::neuron0x860d358() {
   return input4;
}

double NNfunction::neuron0x860d530() {
   return input5;
}

double NNfunction::input0x860d840() {
   double input = -2.59262;
   input += synapse0x85fe830();
   input += synapse0x860d9f0();
   input += synapse0x860da18();
   input += synapse0x860da40();
   input += synapse0x860da68();
   input += synapse0x860da90();
   return input;
}

double NNfunction::neuron0x860d840() {
   double input = input0x860d840();
   return ((input < -709. ? 0. : (1/(1+exp(-input)))) * 1)+0;
}

double NNfunction::input0x860dab8() {
   double input = -0.632405;
   input += synapse0x860dcb0();
   input += synapse0x860dcd8();
   input += synapse0x860dd00();
   input += synapse0x860dd28();
   input += synapse0x860dd50();
   input += synapse0x860dd78();
   return input;
}

double NNfunction::neuron0x860dab8() {
   double input = input0x860dab8();
   return ((input < -709. ? 0. : (1/(1+exp(-input)))) * 1)+0;
}

double NNfunction::input0x860dda0() {
   double input = 2.12136;
   input += synapse0x860df98();
   input += synapse0x860dfc0();
   input += synapse0x860dfe8();
   input += synapse0x860e010();
   input += synapse0x860e038();
   input += synapse0x860e0e8();
   return input;
}

double NNfunction::neuron0x860dda0() {
   double input = input0x860dda0();
   return ((input < -709. ? 0. : (1/(1+exp(-input)))) * 1)+0;
}

double NNfunction::input0x860e110() {
   double input = -0.392621;
   input += synapse0x860e2c0();
   input += synapse0x860e2e8();
   input += synapse0x860e310();
   input += synapse0x860e338();
   input += synapse0x860e360();
   input += synapse0x860e388();
   return input;
}

double NNfunction::neuron0x860e110() {
   double input = input0x860e110();
   return ((input < -709. ? 0. : (1/(1+exp(-input)))) * 1)+0;
}

double NNfunction::input0x860e3b0() {
   double input = -0.0330152;
   input += synapse0x860e5a8();
   input += synapse0x860e5d0();
   input += synapse0x860e5f8();
   input += synapse0x860e620();
   input += synapse0x860e648();
   input += synapse0x860e670();
   return input;
}

double NNfunction::neuron0x860e3b0() {
   double input = input0x860e3b0();
   return ((input < -709. ? 0. : (1/(1+exp(-input)))) * 1)+0;
}

double NNfunction::input0x860e698() {
   double input = 0.353032;
   input += synapse0x860e798();
   input += synapse0x860e7c0();
   input += synapse0x860e7e8();
   input += synapse0x860e060();
   input += synapse0x860e088();
   return input;
}

double NNfunction::neuron0x860e698() {
   double input = input0x860e698();
   return (input * 1)+0;
}

double NNfunction::synapse0x85fe830() {
   return (neuron0x85fe910()*-0.745014);
}

double NNfunction::synapse0x860d9f0() {
   return (neuron0x85feae8()*-0.78933);
}

double NNfunction::synapse0x860da18() {
   return (neuron0x860cfa8()*0.459588);
}

double NNfunction::synapse0x860da40() {
   return (neuron0x860d180()*0.00956551);
}

double NNfunction::synapse0x860da68() {
   return (neuron0x860d358()*2.85689);
}

double NNfunction::synapse0x860da90() {
   return (neuron0x860d530()*6.88496);
}

double NNfunction::synapse0x860dcb0() {
   return (neuron0x85fe910()*2.70722);
}

double NNfunction::synapse0x860dcd8() {
   return (neuron0x85feae8()*-5.22734);
}

double NNfunction::synapse0x860dd00() {
   return (neuron0x860cfa8()*1.95147);
}

double NNfunction::synapse0x860dd28() {
   return (neuron0x860d180()*1.69863);
}

double NNfunction::synapse0x860dd50() {
   return (neuron0x860d358()*-1.4849);
}

double NNfunction::synapse0x860dd78() {
   return (neuron0x860d530()*-0.0587496);
}

double NNfunction::synapse0x860df98() {
   return (neuron0x85fe910()*-0.156646);
}

double NNfunction::synapse0x860dfc0() {
   return (neuron0x85feae8()*0.324913);
}

double NNfunction::synapse0x860dfe8() {
   return (neuron0x860cfa8()*-0.164592);
}

double NNfunction::synapse0x860e010() {
   return (neuron0x860d180()*-0.0524867);
}

double NNfunction::synapse0x860e038() {
   return (neuron0x860d358()*0.415657);
}

double NNfunction::synapse0x860e0e8() {
   return (neuron0x860d530()*20.8707);
}

double NNfunction::synapse0x860e2c0() {
   return (neuron0x85fe910()*0.274578);
}

double NNfunction::synapse0x860e2e8() {
   return (neuron0x85feae8()*-0.0421086);
}

double NNfunction::synapse0x860e310() {
   return (neuron0x860cfa8()*0.465929);
}

double NNfunction::synapse0x860e338() {
   return (neuron0x860d180()*0.536229);
}

double NNfunction::synapse0x860e360() {
   return (neuron0x860d358()*0.365813);
}

double NNfunction::synapse0x860e388() {
   return (neuron0x860d530()*0.126046);
}

double NNfunction::synapse0x860e5a8() {
   return (neuron0x85fe910()*0.400093);
}

double NNfunction::synapse0x860e5d0() {
   return (neuron0x85feae8()*0.44706);
}

double NNfunction::synapse0x860e5f8() {
   return (neuron0x860cfa8()*0.344358);
}

double NNfunction::synapse0x860e620() {
   return (neuron0x860d180()*0.672479);
}

double NNfunction::synapse0x860e648() {
   return (neuron0x860d358()*0.372075);
}

double NNfunction::synapse0x860e670() {
   return (neuron0x860d530()*-0.268089);
}

double NNfunction::synapse0x860e798() {
   return (neuron0x860d840()*1.53795);
}

double NNfunction::synapse0x860e7c0() {
   return (neuron0x860dab8()*-2.55427);
}

double NNfunction::synapse0x860e7e8() {
   return (neuron0x860dda0()*-2.13902);
}

double NNfunction::synapse0x860e060() {
   return (neuron0x860e110()*1.00768);
}

double NNfunction::synapse0x860e088() {
   return (neuron0x860e3b0()*0.838776);
}

