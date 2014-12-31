var initialized = false;
Pebble.addEventListener("ready", function() {
console.log("ready called!");
initialized = true;
});
Pebble.addEventListener("showConfiguration", function() {
var saveOptions = JSON.parse(window.localStorage.getItem("options"));
var url = "http://zckr.com/prieni/revolving_bubbles_config.php?v=1.0";
if(saveOptions !== null) {
url += "&time=" + (saveOptions["revolvingclock0"] ? encodeURIComponent(saveOptions["revolvingclock0"]) : "") +
"&date=" + (saveOptions["revolvingclock1"] ? encodeURIComponent(saveOptions["revolvingclock1"]) : "") +
"&rowd=" + (saveOptions["revolvingclock2"] ? encodeURIComponent(saveOptions["revolvingclock2"]) : "") +
"&invt=" + (saveOptions["revolvingclock3"] ? encodeURIComponent(saveOptions["revolvingclock3"]) : "") +
"&four=" + (saveOptions["revolvingclock4"] ? encodeURIComponent(saveOptions["revolvingclock4"]) : "");
}
console.log("Showing configuration: " + url);
Pebble.openURL(url);
});
Pebble.addEventListener("webviewclosed", function(e) {
console.log("configuration closed");
// webview closed
if(e.response && e.response.length>5) {
var options = JSON.parse(decodeURIComponent(e.response));
console.log("Options = " + JSON.stringify(options));
var saveOptions = {
"revolvingclock0": options["0"],
"revolvingclock1": options["1"],
"revolvingclock2": options["2"],
"revolvingclock3": options["3"],
"revolvingclock4": options["4"]
};
window.localStorage.setItem("options", JSON.stringify(saveOptions));
Pebble.sendAppMessage(options,
function(e) {
console.log("Successfully sent options to Pebble");
},
function(e) {
console.log("Failed to send options to Pebble.\nError: " + e.error.message);
}
);
} else {
console.log("Error with JS Config options received.");	
}
});