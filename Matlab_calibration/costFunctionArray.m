function z=costFunctionArray(theta)
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
    K=diag(theta(2*L+9*(m-1)+1:2*L+9*(m-1)+3));
    b=theta(2*L+9*(m-1)+4:2*L+9*(m-1)+6);
    epsilon=theta(2*L+9*(m-1)+7:2*L+9*(m-1)+9);
    
    y_hat=K*u+K*cross(epsilon,u)+b;
    
    z(m,:,l)=squeeze(cholQinv(m,:,:,l))*(y_meas(m,:,l)'-y_hat);   
    end
  
    
    K=diag(theta(2*L+9*(M-1)+1:2*L+9*(M-1)+3));
    b=theta(2*L+9*(M-1)+4:2*L+9*(M-1)+6);
    y_hat=K*u+b;
    z(M,:,l)=squeeze(cholQinv(M,:,:,l))*(y_meas(M,:,l)'-y_hat);   
    
end

z=reshape(z,3*M*L,1); 
end