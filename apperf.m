# Copyright (C) 2007 Bart Massey
# ALL RIGHTS RESERVED
# 
# This program is licensed under the GPL version 2 or GPL
# version 3 or later.  Please see the file COPYING in the
# source distribution of this software for license terms.

function [x,z] = horner(lower,step,upper,degree)
  x = [lower:step:upper];
  y = erfinv(x);
  fit = polyfit(x,y,degree);
  z = polyval(fit,x);
  printf("if (x <= %g)\n", upper);
  printf("\treturn %.9f + x * ", fit(degree + 1));
  for i = 2:degree
    printf("(%.9f + x * ", fit(degree - i + 2));
  endfor;
  printf("%.9f", fit(1));
  for i = 2:degree
    printf(")");
  endfor;
  printf(";\n");
endfunction;

[x1,z1] = horner(0,0.001,0.4,1);
[x2,z2] = horner(0.4,0.001,0.9,2);
[x3,z3] = horner(0.9,0.001,0.99,3);
[x4,z4] = horner(0.99,0.0001,0.999,3);
x = [x1,x2,x3,x4];
y = erfinv(x);
z = [z1,z2,z3,z4];
plot(x,y,x,z);

#function y = F(x,p)
#  y = p(1) + p(2) * exp(p(3) * x);
#endfunction;
#x = [0.999:0.00001:0.99999];
#y = erfinv(x);
#pin = [1.8,0.005,450];
#leasqr(x(:),y(:),pin,"F",0.001,20000,ones(length(y),1), \
#       0.01*ones(length(pin)),"dfdp", \
#       [0.0001*ones(length(pin),1), 0.1*ones(length(pin),1)])
