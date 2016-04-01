var express = require('express');
var router = express.Router();
var fs = require('fs');

/* GET home page. */
router.get('/', function(req, res,next) {
  	console.log(req);
	res.sendFile('weather.html', { title: 'public' });
});

router.get('/getcity',function(req,res,next) {
	
	console.log("In Getcity");
	console.log(req.query);
	var myRe = new RegExp("^" + req.query.q);
        console.log(myRe);
	fs.readFile(__dirname + '/cities.dat.txt',function(err,data) {
            if(err) throw err;
            var cities = data.toString().split("\n");
            var jsonresult = [];
	    for(var i = 0; i < cities.length; i++) {
            	var result = cities[i].search(myRe);
          	if(result != -1) {
		  jsonresult.push({city:cities[i]});
            	  console.log(cities[i]);
          	}
	    }
	    res.status(200).json(jsonresult);
        })
});

router.get('/getpokemon',function(req,res,next) {

	console.log("In GetPokemon");
	console.log(req.query);
	var namereg;
	var attackreg;
	var defensereg;
	var typereg;
	if(null != req.query.name)
		namereg = new RegExp("^" + req.query.name);
	if(null != req.query.attack)
		attackreg = new RegExp("^" + req.query.attack);
	if(null != req.query.defense)
		defensereg = new RegExp("^" + req.query.defense);
        if(null != req.query.type)
                typereg = new RegExp("^" + req.query.type);

	fs.readFile(__dirname + '/pokemon.json',function(err,data) {
		if(err) throw err;
		var pokestr = data.toString();
		pokestr.trim();
		var pokejson = JSON.parse(pokestr);
		var matches = [];
		console.log(pokejson[1].name);
		for(var i in pokejson) {
			var bool = 1;
			if(namereg != null && pokejson[i].name != req.query.name)
				bool = 0;
			if(attackreg != null && pokejson[i].attack != req.query.attack)
				bool = 0;
			if(defensereg != null && pokejson[i].defense != req.query.defense)
				bool = 0;
			if(typereg != null && pokejson[i].type != req.query.type)
				bool = 0;
			if(bool)
				matches.push(pokejson[i]);
		}
		res.status(200).json(matches);
	});	
});

module.exports = router;
