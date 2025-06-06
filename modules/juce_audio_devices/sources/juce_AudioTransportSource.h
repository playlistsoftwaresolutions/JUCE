/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    An AudioSource that takes a PositionableAudioSource and allows it to be
    played, stopped, started, etc.

    This can also be told use a buffer and background thread to read ahead, and
    if can correct for different sample-rates.

    You may want to use one of these along with an AudioSourcePlayer and AudioIODevice
    to control playback of an audio file.

    @see AudioSource, AudioSourcePlayer

    @tags{Audio}
*/
class JUCE_API  AudioTransportSource  : public PositionableAudioSource,
                                        public ChangeBroadcaster
{
public:
    //==============================================================================
    /** Creates an AudioTransportSource.
        After creating one of these, use the setSource() method to select an input source.
    */
    AudioTransportSource();

    /** Destructor. */
    ~AudioTransportSource() override;

    //==============================================================================
    /** Sets the reader that is being used as the input source.

        This will stop playback, reset the position to 0 and change to the new reader.

        The source passed in will not be deleted by this object, so must be managed by
        the caller.

        @param newSource                        the new input source to use. This may be a nullptr
        @param readAheadBufferSize              a size of buffer to use for reading ahead. If this
                                                is zero, no reading ahead will be done; if it's
                                                greater than zero, a BufferingAudioSource will be used
                                                to do the reading-ahead. If you set a non-zero value here,
                                                you'll also need to set the readAheadThread parameter.
        @param readAheadThread                  if you set readAheadBufferSize to a non-zero value, then
                                                you'll also need to supply this TimeSliceThread object for
                                                the background reader to use. The thread object must not be
                                                deleted while the AudioTransport source is still using it.
        @param sourceSampleRateToCorrectFor     if this is non-zero, it specifies the sample
                                                rate of the source, and playback will be sample-rate
                                                adjusted to maintain playback at the correct pitch. If
                                                this is 0, no sample-rate adjustment will be performed
        @param maxNumChannels                   the maximum number of channels that may need to be played
    */
    void setSource (PositionableAudioSource* newSource,
                    int readAheadBufferSize = 0,
                    TimeSliceThread* readAheadThread = nullptr,
                    double sourceSampleRateToCorrectFor = 0.0,
                    int maxNumChannels = 2);

    //==============================================================================
    /** Changes the current playback position in the source stream.

        The next time the getNextAudioBlock() method is called, this
        is the time from which it'll read data.

        @param newPosition    the new playback position in seconds

        @see getCurrentPosition
    */
    void setPosition (double newPosition);

    /** Returns the position that the next data block will be read from.
        This is a time in seconds.
    */
    double getCurrentPosition() const;

    /** Returns the stream's length in seconds. */
    double getLengthInSeconds() const;

    /** Returns true if the player has stopped because its input stream ran out of data. */
    virtual bool hasStreamFinished() const noexcept;

    //==============================================================================
    /** Starts playing (if a source has been selected).

        If it starts playing, this will send a message to any ChangeListeners
        that are registered with this object.
    */
    void start();

    /** Stops playing.

        If it's actually playing, this will send a message to any ChangeListeners
        that are registered with this object.
    */
    void stop();

    /** Returns true if it's currently playing. */
    bool isPlaying() const noexcept     { return playing; }

    //==============================================================================
    /** Changes the gain to apply to the output.
        @param newGain  a factor by which to multiply the outgoing samples,
                        so 1.0 = 0dB, 0.5 = -6dB, 2.0 = 6dB, etc.
    */
    void setGain (float newGain) noexcept;

    /** Returns the current gain setting.
        @see setGain
    */
    float getGain() const noexcept      { return gain; }

    //==============================================================================
    /** Implementation of the AudioSource method. */
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;

    /** Implementation of the AudioSource method. */
    void releaseResources() override;

    /** Implementation of the AudioSource method. */
    void getNextAudioBlock (const AudioSourceChannelInfo&) override;

    //==============================================================================
    /** Implements the PositionableAudioSource method. */
    void setNextReadPosition (int64 newPosition) override;

    /** Implements the PositionableAudioSource method. */
    int64 getNextReadPosition() const override;

    /** Implements the PositionableAudioSource method. */
    int64 getTotalLength() const override;

    /** Implements the PositionableAudioSource method. */
    bool isLooping() const override;

private:
    //==============================================================================
    PositionableAudioSource* source = nullptr;
    ResamplingAudioSource* resamplerSource = nullptr;
    BufferingAudioSource* bufferingSource = nullptr;
    PositionableAudioSource* positionableSource = nullptr;
    AudioSource* masterSource = nullptr;

    CriticalSection callbackLock;
    float gain = 1.0f, lastGain = 1.0f;
    std::atomic<bool> playing { false }, stopped { true };
    double sampleRate = 44100.0, sourceSampleRate = 0;
    int blockSize = 128, readAheadBufferSize = 0;
    bool isPrepared = false;

    void releaseMasterResources();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioTransportSource)
};

} // namespace juce
