var express = require('express');
var router = express.Router();
var mongoose = require('mongoose');

mongoose.connect('mongodb://localhost/musicDB'); //Connects to a mongo database called "commentDB"

var commentSchema = mongoose.Schema({ //Defines the Schema for this database
  Name: String,
  Comment: String,
  EvilTwin: String
});

var Comment = mongoose.model('Comment', commentSchema); //Makes an object from that schema as a model

var db = mongoose.connection; //Saves the connection as a variable to use
db.on('error', console.error.bind(console, 'connection error:')); //Checks for connection errors
db.once('open', function() { //Lets us know when we're connected
  console.log('Connected');
});

var twinColl = db.collection('twins');

router.post('/comment', function(req, res, next) {
  console.log("POST comment route"); //[1]
  console.log(req.body);

  var newcomment = new Comment(req.body); //[3]
  console.log(newcomment); //[3]

  var newTwin = {Name:req.body.EvilTwin, Comment:req.body.Comment.split("").reverse().join("")};
  twinColl.insert(newTwin, function (err, result) {
    if(err) {
      console.log(err);
    }
    else {
        console.log('Inserted %d documents into the "users" collection. The documents inserted with "_id" are:', result.length, result);
    }
  });

  newcomment.save(function(err, post) { //[4]
    if (err) return console.error(err);
    console.log(post);
    res.sendStatus(200);
  });
});

/* GET comments from database */
router.get('/comment', function(req, res, next) {
  console.log("In the GET route?");
  Comment.find(function(err,commentList) { //Calls the find() method on your database
    if (err) return console.error(err); //If there's an error, print it out
    else {
      console.log(commentList); //Otherwise console log the comments you found
      res.json(commentList);
    }
  })
});

router.get('/twins', function(req,res,next) {
  console.log("GET twins");
  twinColl.find().toArray(function(err,twinList) {
    if(err) return console.error(err);
    else {
      console.log(twinList);
      res.json(twinList);
    }
  });
});

module.exports = router;

