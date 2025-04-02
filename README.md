# growPi
A simple game for Rasp Pi using minimal cairo and Gtk, has a small minimalistic game API. 

growPi -> grow and harvest raspberries before a locust eats your produce. 

So this is a simple 2d game, about as simple as I could make it. I made a minimal API in C that uses GTK and cairo underneath. I
developed it on the Rasp Pi 5 using the Bookworm distro. I don't know why it would not run on any GTk 3.0 Linux system.

The game begins with a farmer who has 20 seeds and 10 small sprouted raspberry plants. He has to plant these in a manner that they grow
and get harvested before a locust eats them. Space and time are important, planting is easy, left mouse click for seeds and right
mouse click for plants. As the game progresses the plants grow automatically and when they get edible the nasty locust will head
for them to start eating them. He can only eat so quickly so the plant will still grow and fruit. When it does the farmer will
head over and pluck the juicy berries. 

After 10 plants grow and die and nothing is left growing the round will end and the farmer will win if he harvests more than
the locust eats. Four more rounds take place and the pace quickens.

It is possible to win every time once you learn how.
