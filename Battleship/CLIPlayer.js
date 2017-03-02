var CLIPlayer = function(game, cli_input, cli_output, map, is_player_one) {

    if (is_player_one) {
	var key = game.registerPlayerOne();
    } else {
	key = game.registerPlayerTwo();
    }

    var powr = game;

    cli_output = $(cli_output);
    cli_input = $(cli_input);
    map = $(map);

    var eventLogHandler = function(e) {
	var cli_msg = $('<div class="cli_msg"></div>');

	switch (e.event_type) {
	case SBConstants.TURN_CHANGE_EVENT:
	    if (e.who == SBConstants.PLAYER_ONE) {
		cli_msg.text("Player one's turn (count = " + game.getTurnCount() + ")");
	    } else {
		cli_msg.text("Player two's turn (count = " + game.getTurnCount() + ")");
	    }
	    break;
	case SBConstants.MISS_EVENT:
	    cli_msg.text("Miss event at (" + e.x + ", " + e.y + ")");
	    break;
	case SBConstants.HIT_EVENT:
	    cli_msg.text("Hit event at (" + e.x + ", " + e.y + ")");
	    break;
	case SBConstants.SHIP_SUNK_EVENT:
	    var ship = e.ship;
	    if (ship.isMine(key)) {
		var pos = ship.getPosition(key);
		cli_msg.text("Foe sunk your " + ship.getName() + " at (" + pos.x + ", " + pos.y + ")");
	    } else {
		var pos = ship.getPosition(null); // This works because ship is dead.
		cli_msg.text("You sunk their " + ship.getName() + " at (" + pos.x + ", " + pos.y + ")");
	    }
	    break;
	case SBConstants.GAME_OVER_EVENT:
	    if (is_player_one && e.winner == SBConstants.PLAYER_ONE) {
		cli_msg.text("Game over. You win!");
	    } else {
		cli_msg.text("Game over. You lose!");
	    }
	    break;
	}
	cli_output.prepend(cli_msg);
    };

    game.registerEventHandler(SBConstants.TURN_CHANGE_EVENT,
			      eventLogHandler);
    game.registerEventHandler(SBConstants.MISS_EVENT,
			      eventLogHandler);
    game.registerEventHandler(SBConstants.HIT_EVENT,
			      eventLogHandler);
    game.registerEventHandler(SBConstants.SHIP_SUNK_EVENT,
			      eventLogHandler);
    game.registerEventHandler(SBConstants.GAME_OVER_EVENT,
            eventLogHandler);

    var mapDrawHandler = function(e) {
	$('#map_view').empty();
  $('#map_view').append('<table id="board">');
  $('#map_view').append('<tbody>');


	for (var y=0; y<game.getBoardSize(); y++) {
	    $('#map_view').append('<tr>');
	    for (var x=0; x<game.getBoardSize(); x++) {
		var sqr = game.queryLocation(key, x, y);
		switch (sqr.type) {
		case "miss":
		    $('#map_view').append("<td id='"+x+"_"+y+"' bgcolor='#a67e05'></td>");
		    break;
		case "p1":
		    if (sqr.state == SBConstants.OK) {
			$('#map_view').append("<td id='"+x+"_"+y+"' bgcolor='#00FFe5'></td>");
		    } else {
			$('#map_view').append("<td id='"+x+"_"+y+"' bgcolor='#ff4336'></td>");
		    }
		    break;
		case "p2":
		    if (sqr.state == SBConstants.OK) {
			$('#map_view').append("<td id='"+x+"_"+y+"' bgcolor='#ff216b'></td>");
		    } else {
			$('#map_view').append("<td id='"+x+"_"+y+"' bgcolor='#a62c23'></td>");
		    }
		    break;
		case "empty":
		    $('#map_view').append("<td id='"+x+"_"+y+"' bgcolor='#005a66'></td>");
		    break;
		case "invisible":
		    $('#map_view').append("<td id='"+x+"_"+y+"' bgcolor='#00444d'></td>");
		    break;
		}
	    }
	    $('#map_view').append('</tr>');
	}
  $('#map_view').append('</tbody>');
  $('#map_view').append('</table>');
    };



    game.registerEventHandler(SBConstants.TURN_CHANGE_EVENT,
			      mapDrawHandler);


      $("#map_view").delegate('td', 'click', function(){
        var coord = $(this).attr('id').split('_');
        var x = parseInt(coord[0]);
        var y = parseInt(coord[1]);
        game.shootAt(key, x, y);

      });


    $('#rcw').on('click', function(){
      var select = $('#ships').find(":selected").text();
      var ship = game.getShipByName(key, select);
      if (ship != null) {
          game.rotateShipCW(key, ship);
      }
    })
    $('#rccw').on('click', function(){
      var select = $('#ships').find(":selected").text();
      var ship = game.getShipByName(key, select);
      if (ship != null) {
          game.rotateShipCCW(key, ship);
      }
    })
    $('#fwd').on('click', function(){
      var select = $('#ships').find(":selected").text();
      var ship = game.getShipByName(key, select);
      if (ship != null) {
          game.moveShipForward(key, ship);
      }
    })
    $('#bwd').on('click', function(){
      var select = $('#ships').find(":selected").text();
      var ship = game.getShipByName(key, select);
      if (ship != null) {
          game.moveShipBackward(key, ship);
      }
    })
    $('#info').on('click', function(){
        var fleet = game.getFleetByKey(key);
        var fleet_ul = $('<ul></ul>');
        fleet.forEach(function (s) {
          var ship_str = "<li>" + s.getName();
          var ship_pos = s.getPosition(key);
          ship_str += "<ul>";
          ship_str += "<li>Position: " + ship_pos.x + ", " + ship_pos.y + "</li>";
          ship_str += "<li>Direction: " + ship_pos.direction + "</li>";
          ship_str += "<li>Size: " + s.getSize() + "</li>";
            if (s.getStatus() == SBConstants.ALIVE) {
              ship_str += "<li>Status: ALIVE</li>";
            } else {
              ship_str += "<li>Status: DEAD</li>";
            }
          ship_str += "</ul></li>";
          fleet_ul.append(ship_str);
        })
        cli_output.prepend($('<div class="cli_msg"></div>').append(fleet_ul));
    })

};
