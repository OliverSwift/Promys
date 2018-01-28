
function active(platform) {
	document.getElementById(platform).style.display = 'inline';
}

function platform() {
	if(navigator.userAgent.indexOf('Linux') != -1) {
		active('linux');
	} else if(navigator.userAgent.indexOf('Macintosh') != -1) {
		active('macos');
	} else if(navigator.userAgent.indexOf('Windows NT') != -1) {
		active('windows');
	} else {
		active('windows');
		active('macos');
		active('linux');
	}
}

