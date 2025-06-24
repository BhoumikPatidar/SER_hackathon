"""
ElevenLabs Emotional Speech Dataset Generator

This script generates emotional speech samples using ElevenLabs Eleven v3 Audio Tags.
Audio Tags like [angry], [excited], [sad] provide emotional direction WITHOUT being spoken aloud.

Requirements:
- ElevenLabs API key in .env file
- Access to Eleven v3 model (currently in alpha)

Features:
- 7 emotions: happy, anger, disgust, neutral, fear, surprise, sad
- 10 diverse voice IDs for speaker variety
- Audio Tags for precise emotional control
- Optimized voice settings for v3 model
"""

from dotenv import load_dotenv
from elevenlabs.client import ElevenLabs
import os
import random
import time

load_dotenv()

# Initialize ElevenLabs client
elevenlabs = ElevenLabs(api_key=os.getenv("ELEVENLABS_API_KEY"))

# Diverse voice IDs for speaker variety (common default voices)
VOICE_IDS = [
    "JBFqnCBsd6RMkjVDRZzb",  # George - British Male
    "21m00Tcm4TlvDq8ikWAM",  # Rachel - American Female  
    "AZnzlk1XvdvUeBnXmlld",  # Domi - American Female
    "EXAVITQu4vr4xnSDxMaL",  # Bella - American Female
    "ErXwobaYiN019PkySvjV",  # Antoni - American Male
    "MF3mGyEYCl7XYWbV9V6O",  # Elli - American Female
    "TxGEqnHWrfWFTfGW9XjX",  # Josh - American Male
    "VR6AewLTigWG4xSOukaG",  # Arnold - American Male
    "pNInz6obpgDQGcFmaJgB",  # Adam - American Male
    "yoZ06aMxZJJ28mfd3POQ",  # Sam - American Male
]

# Emotional text templates with Audio Tags - pure speech content with emotional direction
# Audio Tags like [angry], [sad] are performance cues that control emotion WITHOUT being spoken
EMOTIONAL_TEXTS = {
    "happy": [
        "[excited] I can't believe it! This is the best day ever!",
        "[cheerfully] Amazing! Everything is working out perfectly!",
        "[joyfully] Fantastic! I couldn't be happier about this!",
        "[laughs] This is incredible! I'm so thrilled right now!",
        "[enthusiastically] Excellent! This success feels absolutely wonderful!",
        "[excited] Perfect! Everything is falling into place beautifully!",
        "[cheerfully] Brilliant! This celebration is well deserved!",
        "[joyfully] This is beyond my wildest dreams! So exciting!",
        "[laughs] Wonderful! Today is truly spectacular and amazing!",
        "[enthusiastically] Outstanding! This fantastic news made my entire week!"
    ],
    
    "anger": [
        "[angrily] This is completely unacceptable! I've had enough!",
        "[furiously] How dare you do this to me! This is outrageous!",
        "[angrily] I can't stand this anymore! This stops right now!",
        "[furiously] You've crossed the line! This behavior is inappropriate!",
        "[angrily] This incompetence is absolutely infuriating! Fix it!",
        "[furiously] I won't tolerate this! Your actions are inexcusable!",
        "[angrily] This is ridiculous! Stop this nonsense immediately!",
        "[furiously] This betrayal makes my blood boil! Unbelievable!",
        "[angrily] Your negligence is completely unacceptable! Enough!",
        "[furiously] This disgusting behavior ends now! I'm done!"
    ],
    
    "disgust": [
        "[disgusted] Ugh, that's absolutely revolting! I can't even look.",
        "[revolted] This is completely nauseating and totally repulsive.",
        "[disgusted] That's disgusting! How can anyone tolerate this mess?",
        "[revolted] Gross! This smell is making me feel sick.",
        "[disgusted] This is utterly repugnant and completely disturbing.",
        "[revolted] That's vile! This sight is absolutely sickening.",
        "[disgusted] This is repulsive! I feel nauseous just thinking about it.",
        "[revolted] This is horrifyingly grotesque and completely appalling.",
        "[disgusted] This is beyond disgusting! Absolutely awful!",
        "[revolted] This putrid mess is completely unbearable! Horrible!"
    ],
    
    "neutral": [
        "The weather forecast shows partly cloudy skies with temperatures around seventy degrees.",
        "Please remember to submit your quarterly reports by five PM this Friday.",
        "The meeting is scheduled for Tuesday afternoon at two thirty in the conference room.",
        "Our office hours are Monday through Friday, nine AM to six PM.",
        "The project deadline has been extended to the end of next month.",
        "Today's agenda includes reviewing the budget and discussing new policies.",
        "The parking lot will be closed for maintenance from eight to ten AM.",
        "Please update your contact information in the employee database system.",
        "The training session will cover safety procedures and emergency protocols.",
        "All documents must be reviewed and approved before final publication."
    ],
    
    "fear": [
        "[fearfully] I think someone's following us... We need to get out of here now!",
        "[nervously] Something terrible is about to happen... I can feel it coming.",
        "[scared] I'm scared... What if we don't make it out alive?",
        "[nervously] This place gives me chills... Something's not right here at all.",
        "[fearfully] I have a horrible feeling about this entire situation...",
        "[scared] Help me! I don't think I can handle this alone anymore!",
        "[fearfully] I'm terrified... What if they find us hiding here?",
        "[panicked] We're in serious danger... We need to run right now!",
        "[scared] I can't breathe... This nightmare feels way too real...",
        "[nervously] Something's wrong... I hear strange noises in the darkness..."
    ],
    
    "surprise": [
        "[surprised] What?! I never expected this to happen at all!",
        "[shocked] No way! This is completely unexpected and shocking!",
        "[amazed] Really?! I can't believe you actually did it!",
        "[astonished] Seriously?! This changes everything completely!",
        "[surprised] Oh my goodness! This is such an amazing surprise!",
        "[shocked] Incredible! I never saw this plot twist coming!",
        "[amazed] What a shock! This revelation is absolutely mind-blowing!",
        "[astonished] Unbelievable! This outcome defies all my expectations!",
        "[surprised] Wow! This discovery is truly remarkable and stunning!",
        "[shocked] Amazing! This surprise left me completely speechless!"
    ],
    
    "sad": [
        "[sadly] I miss them so much... This pain feels unbearable.",
        "[mournfully] This loss feels like a huge hole in my heart...",
        "[sadly] It's over... Everything we built together is gone forever.",
        "[sorrowfully] I feel so alone... Nothing will ever be the same again.",
        "[sadly] This goodbye is much harder than I ever imagined...",
        "[mournfully] I'm heartbroken... This pain feels completely unbearable right now.",
        "[sadly] Why did this happen? Life can be so cruel sometimes...",
        "[sorrowfully] I don't know how to move forward from this tragedy...",
        "[sadly] It hurts so much... This emptiness is overwhelming me...",
        "[mournfully] I'm devastated... This has changed everything in my life forever..."
    ]
}

# Voice settings optimized for Eleven v3 audio tags
EMOTION_SETTINGS = {
    "happy": {"stability": 0.4, "similarity_boost": 0.8, "style": 0.6},       # Creative for expressiveness
    "anger": {"stability": 0.3, "similarity_boost": 0.9, "style": 0.7},       # Creative for strong emotion
    "disgust": {"stability": 0.4, "similarity_boost": 0.8, "style": 0.6},     # Balanced for revulsion
    "neutral": {"stability": 0.6, "similarity_boost": 0.7, "style": 0.0},     # Natural for neutrality
    "fear": {"stability": 0.3, "similarity_boost": 0.8, "style": 0.7},        # Creative for anxiety
    "surprise": {"stability": 0.2, "similarity_boost": 0.9, "style": 0.8},    # Very creative for shock
    "sad": {"stability": 0.5, "similarity_boost": 0.8, "style": 0.4}          # Natural for melancholy
}

def generate_emotional_dataset(samples_per_emotion=10):
    """
    Generate emotional speech dataset with maximum variability and emotional accuracy.
    
    Args:
        samples_per_emotion: Number of samples to generate per emotion (default: 10)
    """
    
    total_samples = len(EMOTIONAL_TEXTS) * samples_per_emotion
    print(f"🎯 Generating {total_samples} emotional speech samples ({samples_per_emotion} per emotion)")
    print("📊 Emotions: happy, anger, disgust, neutral, fear, surprise, sad")
    print("🎤 Using diverse voices for speaker variability")
    print("🏷️ Using Eleven v3 Audio Tags for precise emotional control")
    print("=" * 60)
    
    # Create dataset directory
    dataset_dir = "emotional_speech_dataset"
    os.makedirs(dataset_dir, exist_ok=True)
    
    sample_count = 0
    
    for emotion, texts in EMOTIONAL_TEXTS.items():
        print(f"\n🎭 Generating {emotion.upper()} samples...")
        
        # Create emotion directory
        emotion_dir = os.path.join(dataset_dir, emotion)
        os.makedirs(emotion_dir, exist_ok=True)
        
        for i in range(samples_per_emotion):
            try:
                # Select random text and voice for variety
                text = random.choice(texts)
                voice_id = random.choice(VOICE_IDS)
                
                # Get emotion-specific settings
                voice_settings = EMOTION_SETTINGS[emotion].copy()
                
                # Add slight randomization to settings for more variety
                voice_settings["stability"] += random.uniform(-0.1, 0.1)
                voice_settings["similarity_boost"] += random.uniform(-0.05, 0.05)
                
                # Clamp values to valid ranges
                voice_settings["stability"] = max(0.0, min(1.0, voice_settings["stability"]))
                voice_settings["similarity_boost"] = max(0.0, min(1.0, voice_settings["similarity_boost"]))
                
                print(f"  📝 Sample {i+1}: {text[:50]}...")
                print(f"  🎤 Voice: {voice_id[:8]}... | Settings: {voice_settings}")
                
                # Generate audio using Eleven v3 for audio tags support
                audio = elevenlabs.text_to_speech.convert(
                    text=text,
                    voice_id=voice_id,
                    model_id="eleven_v3",  # Required for audio tags functionality
                    output_format="mp3_44100_128",
                    voice_settings=voice_settings
                )
                
                # Save as MP3
                mp3_filename = f"{emotion}_{i+1:02d}_{voice_id[:8]}.mp3"
                mp3_path = os.path.join(emotion_dir, mp3_filename)
                
                with open(mp3_path, "wb") as f:
                    for chunk in audio:
                        f.write(chunk)
                
                sample_count += 1
                print(f"  ✅ Saved: {mp3_filename}")
                
                # Brief pause to avoid rate limiting
                time.sleep(0.5)
                
            except Exception as e:
                print(f"  ❌ Error generating {emotion} sample {i+1}: {e}")
                continue
    
    print("\n" + "=" * 60)
    print(f"🎉 Dataset generation complete!")
    print(f"📁 Location: {os.path.abspath(dataset_dir)}")
    print(f"📊 Total samples created: {sample_count}")
    print(f"🎭 Emotions covered: {len(EMOTIONAL_TEXTS)}")
    print(f"🎤 Voice variety: {len(VOICE_IDS)} different speakers")
    
    # Print summary
    print(f"\n📋 Dataset Summary:")
    for emotion in EMOTIONAL_TEXTS.keys():
        emotion_dir = os.path.join(dataset_dir, emotion)
        if os.path.exists(emotion_dir):
            count = len([f for f in os.listdir(emotion_dir) if f.endswith('.mp3')])
            print(f"  {emotion.capitalize()}: {count} samples")

def test_single_emotion(emotion="happy", sample_idx=0):
    """Test generation of a single emotional sample"""
    
    if emotion not in EMOTIONAL_TEXTS:
        print(f"❌ Invalid emotion. Choose from: {list(EMOTIONAL_TEXTS.keys())}")
        return
    
    print(f"🧪 Testing {emotion} emotion with Audio Tags...")
    print("⚠️ Note: This requires Eleven v3 model access (currently alpha)")
    
    text = EMOTIONAL_TEXTS[emotion][sample_idx]
    voice_id = VOICE_IDS[0]  # Use first voice for testing
    voice_settings = EMOTION_SETTINGS[emotion]
    
    print(f"📝 Text: {text}")
    print(f"🎤 Voice: {voice_id}")
    print(f"⚙️ Settings: {voice_settings}")
    
    try:
        audio = elevenlabs.text_to_speech.convert(
            text=text,
            voice_id=voice_id,
            model_id="eleven_v3",  # Required for audio tags
            output_format="mp3_44100_128",
            voice_settings=voice_settings
        )
        
        # Save test file
        with open(f"test_{emotion}.mp3", "wb") as f:
            for chunk in audio:
                f.write(chunk)
        
        print(f"✅ Test complete! Saved as test_{emotion}.mp3")
        
    except Exception as e:
        print(f"❌ Test failed: {e}")

if __name__ == "__main__":
    print("🎭 ElevenLabs Emotional Speech Dataset Generator")
    print("🏷️ Using Eleven v3 Audio Tags for precise emotional control")
    print("=" * 60)
    
    # Uncomment to test a single emotion first
    # test_single_emotion("happy")
    
    # Generate full dataset (10 samples per emotion = 70 total)
    generate_emotional_dataset(samples_per_emotion=10)