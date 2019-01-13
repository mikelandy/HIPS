%
% scale a matlab variable and display it
% This version pipes it out using hips2
%
function dispm(b)
	maxval = max(max(b))
	minval = min(min(b))
	a = (b - minval) * 255 / (maxval - minval);
	hips2('mhips',a,1,0);
end
