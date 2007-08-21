format long;
x = [0:0.01:0.9];
y = erfinv(x);
polyfit(x,y,3)
x = [0.9:0.001:0.99];
y = erfinv(x);
polyfit(x,y,3)
#x = [0.99:0.0001:0.999];
#y = erfinv(x);
#function y = F(x,p)
#  y = p(1) + p(2) * exp(p(3) * x);
#endfunction;
#leasqr(x(:),y(:),[1 1 1],"F",0.0001,20000,ones(length(y),1), \
#       0.01*ones(3),"dfdp", \
#       [0.0001*ones(3,1), 0.1*ones(3,1)])
