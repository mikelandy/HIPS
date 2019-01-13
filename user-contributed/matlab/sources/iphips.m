%
% This program calls an arbitrary hips command
% it calls hips2.mex4('command ',image,inflag,outflag)
% command is the hips command name
% image is a matlab matrix 
% inflag is 0: no input to hips routine (e.g. seeheader)
% outflag is 0: no output from hips routine, runs in background (e.g.xhipsc2)
%
function outim = iphips(cmd,im,outflg)
	if nargin == 1
		outim = hips2(cmd,[],0,1);
	elseif nargin == 2
		outim = hips2(cmd,im,1,1);
	elseif nargin == 3
		outim = hips2(cmd,im,1,outflag);
	else 
		disp('bad arguments')
		outim = []
	end
end
