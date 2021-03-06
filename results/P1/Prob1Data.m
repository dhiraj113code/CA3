function Prob1Data(X1, YMatrix1)
%CREATEFIGURE1(X1,YMATRIX1)
%  X1:  vector of x data
%  YMATRIX1:  matrix of y data

%  Auto-generated by MATLAB on 06-Nov-2013 20:43:23

% Create figure
figure1 = figure('Name','Problem 1 : Data Cache');

% Create axes
axes1 = axes('Parent',figure1,'XScale','log','XMinorTick','on');
box(axes1,'on');
hold(axes1,'all');

% Create multiple lines using matrix input to semilogx
semilogx1 = semilogx(X1,YMatrix1,'Parent',axes1,'LineWidth',3);
set(semilogx1(1),'Marker','o','DisplayName','spice.trace');
set(semilogx1(2),'Marker','square','DisplayName','cc.trace');
set(semilogx1(3),'Marker','x','DisplayName','tex.trace');

% Create xlabel
xlabel({'Cache Size (Bytes)'});

% Create ylabel
ylabel({'Hit Rate(%)'});

% Create title
title({'For Data Cache : Hit Rate vs Cache Size','(Block size = 4B, Write back, Write allocate, Fully Associative)'});

% Create legend
legend1 = legend(axes1,'show');
set(legend1,'Location','SouthEast');

