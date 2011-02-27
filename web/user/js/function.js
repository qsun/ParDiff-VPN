$(document).ready(function() {
						   $("#menu-frontpage").click(function(id, ev) {
						       $(".content_show").removeClass("content_show").addClass("content_hidden").hide();
						       $("#frontpage").removeClass("content_hidden").addClass("content_show").fadeTo("slow", 1.0);;
						   	});
							$("#menu-login").click(function(id, ev) {
						       $(".content_show").removeClass("content_show").addClass("content_hidden").hide();
						       $("#login").removeClass("content_hidden").addClass("content_show").fadeTo("slow", 1.0);
						   	});
						   $("#menu-tutorial").click(function(id, ev) {
						       $(".content_show").removeClass("content_show").addClass("content_hidden").hide();
						       $("#tutorial").removeClass("content_hidden").addClass("content_show").fadeTo("slow", 1.0);;
						   	});
						   $("#menu-sale").click(function(id, ev) {
						   	    $(".content_show").removeClass("content_show").addClass("content_hidden").hide();
						   	    $("#sale").removeClass("content_hidden").addClass("content_show").fadeTo("slow", 1.0);;
						   	});
						   });