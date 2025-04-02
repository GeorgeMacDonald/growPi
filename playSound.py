import pygame
from pygame import mixer
import time

# Initialize Pygame mixer
mixer.init()

# Load a sound file
sound = mixer.Sound('sample.wav')

# Play the sound
sound.play()

# Keep the script running for a few seconds to allow the sound to finish playing
time.sleep(5)

# Quit Pygame (optional)
pygame.quit()
