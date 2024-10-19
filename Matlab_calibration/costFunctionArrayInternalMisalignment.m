function z=costFunctionArrayInternalMisalignment(theta)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%>                                                                         
%>  Cost function used in the calibration of the IMU array. Note that
%>  the nonlinear least square optimizer in Matlab requires the cost
%>  function to return the individual errors and note the squared sume of
%>  the errors. 
%>  
%>  Output:     z       A 3*M*L long vector with all the individual errors    
%>  Input:      theta   A 9*M-3 long vector with the optimization
%>                      parameters
%>
%>  @authors Isaac Skog, John-Olof Nilsson
%>  @copyright Copyright (c) 2013 OpenShoe, ISC License (open source)
%>
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%



global y_meas;
global cholQinv;
global g_local;

[M,~,L]=size(y_meas);

z=zeros(M,3,L);

for l=1:L
    
    % Estimate the roll and pitch at the l:th orientation and construct the 
    % input vector u^(i)_n.
    roll=theta(2*(l-1)+1);
    pitch=theta(2*(l-1)+2);
    u=g_local*[ -sin(pitch) cos(pitch)*sin(roll) cos(pitch)*cos(roll)]';
    
    % Calculate the difference between the estimated and measured output of
    % the M IMUs. 
    
    for m=1:M-1
    K=diag(theta(2*L+12*(m-1)+1:2*L+12*(m-1)+3));
    S=eye(3);
    S(1,2)=-theta(2*L+12*(m-1)+7);
    S(1,3)=theta(2*L+12*(m-1)+8);
    S(2,3)=-theta(2*L+12*(m-1)+9);
    b=theta(2*L+12*(m-1)+4:2*L+12*(m-1)+6);
    
    E=eye(3);
    E(1,2)=theta(2*L+12*(m-1)+12);
    E(1,3)=-theta(2*L+12*(m-1)+11);
    E(2,1)=-theta(2*L+12*(m-1)+12);
    E(2,3)=theta(2*L+12*(m-1)+10);
    E(3,1)=theta(2*L+12*(m-1)+11);
    E(3,2)=-theta(2*L+12*(m-1)+10);
    
    y_hat=K*S*E*u+b;
    
    
    z(m,:,l)=squeeze(cholQinv(m,:,:,l))*(y_meas(m,:,l)'-y_hat);   
    end
  
    K=diag(theta(2*L+12*(M-1)+1:2*L+12*(M-1)+3));
    b=theta(2*L+12*(M-1)+4:2*L+12*(M-1)+6);
    S=eye(3);
    S(1,2)=-theta(2*L+12*(M-1)+7);
    S(1,3)=theta(2*L+12*(M-1)+8);
    S(2,3)=-theta(2*L+12*(M-1)+9);
    
    y_hat=K*S*u+b;
    z(M,:,l)=squeeze(cholQinv(M,:,:,l))*(y_meas(M,:,l)'-y_hat);   
    
end

z=reshape(z,3*M*L,1); 
end