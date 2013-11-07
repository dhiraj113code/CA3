S = importdata('P3.spice.result');
C = importdata('P3.cc.result');
T = importdata('P3.tex.result');


Instr_S = 782764;
Data_S = 217237;
Instr_C = 757341;
Data_C = 242661;
Instr_T = 597309;
Data_T = 235168;


%Instruction Cache Size
S_is = S(:,1);
C_is = C(:,1);
T_is = T(:,1);

%Data Cache Size
S_ds = S(:,2);
C_ds = C(:,2);
T_ds = T(:,2);

%Block size
S_b = S(:,3);
C_b = C(:,3);
T_b = T(:,3);

%Associativity
S_a = S(:,4);
C_a = C(:,4);
T_a = T(:,4);

%Instruction Misses & Instruciton Replacements
S_im = S(:,5);
S_ir = S(:,6);
C_im = C(:,5);
C_ir = C(:,6);
T_im = T(:,5);
T_ir = T(:,6);

%Data Misses & Data Replacements
S_dm = S(:,7);
S_dr = S(:,8);
C_dm = C(:,7);
C_dr = C(:,8);
T_dm = T(:,7);
T_dr = T(:,8);

%Demand Fetches
S_df = S(:,9);
C_df = C(:,9);
T_df = T(:,9);

%Copies Back
S_cb = S(:,10);
C_cb = C(:,10);
T_cb = T(:,10);

%Problem 1 Plots:
%Instrucion Hit Rate
S_ihr = (Instr_S - S_im)*100/Instr_S;
C_ihr = (Instr_C - C_im)*100/Instr_C;
T_ihr = (Instr_T - T_im)*100/Instr_T;
%Data Hit Rate
S_dhr = (Data_S - S_dm)*100/Data_S;
C_dhr = (Data_C - C_dm)*100/Data_C;
T_dhr = (Data_T - T_dm)*100/Data_T;


Prob3Instr(S_a, [S_ihr, C_ihr, T_ihr]);
Prob3Data(S_a, [S_dhr, C_dhr, T_dhr]);
