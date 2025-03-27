
function active(platform) {
	document.getElementById(platform).style.display = 'inline';
}

function platform() {
	if (document.location.search != '') {
		active('windows');
		active('macos');
		active('ubuntu');
		active('fedora');
		return;
	}

	if(navigator.userAgent.indexOf('Linux') != -1) {
		active('ubuntu');
		active('fedora');
	} else if(navigator.userAgent.indexOf('Macintosh') != -1) {
		active('macos');
	} else if(navigator.userAgent.indexOf('Windows NT') != -1) {
		active('windows');
	} else {
		active('windows');
		active('macos');
		active('ubuntu');
		active('fedora');
	}
}

