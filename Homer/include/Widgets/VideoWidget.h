/*****************************************************************************
 *
 * Copyright (C) 2008 Thomas Volkert <thomas@homer-conferencing.com>
 *
 * This software is free software.
 * Your are allowed to redistribute it and/or modify it under the terms of
 * the GNU General Public License version 2 as published by the Free Software
 * Foundation.
 *
 * This source is published in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License version 2 for more details.
 *
 * You should have received a copy of the GNU General Public License version 2
 * along with this program. Otherwise, you can write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111, USA.
 * Alternatively, you find an online version of the license text under
 * http://www.gnu.org/licenses/gpl-2.0.html.
 *
 *****************************************************************************/

/*
 * Purpose: widget for video display
 * Since:   2008-12-01
 */

#ifndef _VIDEO_WIDGET_
#define _VIDEO_WIDGET_

#include <HBTime.h>

#include <QImage>
#include <QWidget>
#include <QDockWidget>
#include <QAction>
#include <QMenu>
#include <QTimer>
#include <QThread>
#include <QTime>
#include <QList>
#include <QMutex>
#include <QWaitCondition>
#include <QContextMenuEvent>
#include <QKeyEvent>
#include <QMainWindow>
#include <QFocusEvent>
#include <QStringList>

#include <MediaSource.h>
#include <MeetingEvents.h>
#include <MediaFilterSystemState.h>

#include <MediaSourceGrabberThread.h>

namespace Homer { namespace Gui {

using namespace Homer::Multimedia;

class ParticipantWidget;

///////////////////////////////////////////////////////////////////////////////

// debug performance of video widget: background repainting
//#define DEBUG_VIDEOWIDGET_PERFORMANCE

//#define DEBUG_VIDEOWIDGET_FRAME_DELIVERY

// de/activate automatic frame dropping in case the video widget is invisible (default is off)
//#define VIDEO_WIDGET_DROP_WHEN_INVISIBLE

// de/activate frame handling
//#define VIDEO_WIDGET_DEBUG_FRAMES

// de/activate fullscreen display of mute state
//#define VIDEO_WIDGET_SHOW_MUTE_STATE_IN_FULLSCREEN

///////////////////////////////////////////////////////////////////////////////

#define FRAME_BUFFER_SIZE                                                   3

class VideoWorkerThread;

class VideoWidget:
    public QWidget
{
    Q_OBJECT;

public:
    VideoWidget(QWidget* pParent = NULL);

    virtual ~VideoWidget();

    void Init(QMainWindow* pMainWindow, ParticipantWidget* pParticipantWidget,  MediaSource *pVideoSource, QMenu *pVideoMenu, QString pName = "Video", bool pVisible = false);

    void SetVisible(bool pVisible);
    void SetResolution(int mX, int mY);

    void InformAboutNewFrame();
    void InformAboutOpenError(QString pSourceName);
    void InformAboutNewSource();
    void InformAboutNewSourceResolution();
    void InformAboutSeekingComplete();
    void InformAboutNewFullScreenDisplay();

    VideoWorkerThread* GetWorker();

    QStringList GetVideoInfo();

    void InitializeMenuVideoSettings(QMenu *pMenu);

    /* mosaic mode */
    void ToggleMosaicMode(bool pActive);

    /* fullscreen mode */
    void ToggleFullScreenMode(bool pActive);
    bool IsFullScreen();

public slots:
    void ToggleVisibility();
    void SelectedMenuVideoSettings(QAction *pAction);

private slots:
    void ShowHourGlass();

private:
    void SendActivityToSystem(); // informs the system that there is still activity and screensaver/sleep mode isn't needed, has to be called periodically
    void DialogAddNetworkSink();
    void ShowFrame(void* pBuffer);
    void SetScaling(float pVideoScaleFactor);
    bool IsCurrentScaleFactor(float pScaleFactor);
    void SetResolutionFormat(VideoFormat pFormat);
    void ToggleInVideoSystemState();
    void ToggleSmoothPresentationMode();
    void SavePicture();
    void StartRecorder(bool pQuickRecording = false);
    void StopRecorder();
    void ShowFullScreen(int &pPosX, int &pPosY); // returns the upper left corner where fullscreen starts
    bool SetOriginalResolution();

    /* status message per OSD text */
    void ShowOsdMessage(QString pText);

    virtual void contextMenuEvent(QContextMenuEvent *event);
    virtual void dragEnterEvent(QDragEnterEvent *pEvent);
    virtual void dropEvent(QDropEvent *pEvent);
    virtual void paintEvent(QPaintEvent *pEvent);
    virtual void resizeEvent(QResizeEvent *pEvent);
    virtual void keyPressEvent(QKeyEvent *pEvent);
    virtual void keyReleaseEvent(QKeyEvent *pEvent);
    virtual void mouseDoubleClickEvent(QMouseEvent *pEvent);
    virtual void closeEvent(QCloseEvent* pEvent);
    virtual void customEvent (QEvent* pEvent);
    virtual void wheelEvent(QWheelEvent *pEvent);
    virtual void mouseMoveEvent (QMouseEvent *pEvent);
    virtual void mousePressEvent(QMouseEvent *pEvent);
    virtual void mouseReleaseEvent(QMouseEvent *pEvent);
    virtual void focusOutEvent(QFocusEvent *pEvent);
    virtual void timerEvent(QTimerEvent *pEvent);

    /* user activity handling */
    void FullscreenMarkUserActive();
    void FullscreenMarkUserIdle();

    QWidget             *mCurrentApplicationFocusedWidget;
    QImage              mCurrentFrame;
    int 				mCurrentFrameOutputWidth;
    int 				mCurrentFrameOutputHeight;
    QPoint              mWinPos;
    QAction             *mAssignedAction;
    QMainWindow			*mMainWindow;
    QString             mVideoTitle;
    int                 mUpdateTimerId;
    int                 mResX;
    int                 mResY;
    float				mVideoScaleFactor; // 0.5, 1.0, 1.5, 2.0
    bool                mShowLiveStats;
    int                 mCustomEventReason;
    QTimer              *mHourGlassTimer;
    qreal               mHourGlassAngle;
    int                 mHourGlassOffset;
    bool                mSystemStatePresentation;
    bool                mSmoothPresentation;
    bool                mRecorderStarted;
    bool                mInsideDockWidget;
    bool                mVideoPaused;
    int                 mAspectRatio;
    bool                mNeedBackgroundUpdate, mNeedBackgroundUpdatesUntillNextFrame;
    bool				mVideoMirroredHorizontal;
    bool				mVideoMirroredVertical;
    int                 mLastFrameNumber;
    VideoWorkerThread   *mVideoWorker;
    int                 mPendingNewFrameSignals;
    ParticipantWidget   *mParticipantWidget;
    int64_t 			mPaintEventCounter;
    /* media source */
    int                 mVideoSourceDARHoriz, mVideoSourceDARVert;
    MediaSource         *mVideoSource;
    /* statistics */
    int                 mCurrentFrameNumber;
    float				mCurrentFrameRate;
    /* live marker - OSD */
    bool                mLiveMarkerActive;
    /* status messages per OSD text */
    QString				mOsdStatusMessage;
    int64_t				mOsdStatusMessageTimeout;
    /* mouse hiding */
    QTime               mTimeOfLastMouseMove;
    /* periodic tasks */
    int                 mTimerId;
    /* moving main window per mouse move */
    QPoint				mMovingMainWindowReferencePos;
    bool				mIsMovingMainWindow;
    /* periodic widget updates */
    QTime				mTimeLastWidgetUpdate;
    /* Mosaic mode */
    bool				mMosaicMode;
    /* in-video system state */
    MediaFilterSystemState *mMediaFilterSystemState;
};


class VideoWorkerThread:
    public MediaSourceGrabberThread
{
    Q_OBJECT;
public:
    VideoWorkerThread(QString pName, MediaSource *pVideoSource, VideoWidget *pVideoWidget);

    virtual ~VideoWorkerThread();

    virtual void run();

    /* fullscreen display */
    void SetFullScreenDisplay();

    /* forwarded interface to media source */
    void SetGrabResolution(int pX, int pY);
    void GetGrabResolution(int &pX, int &pY);

    /* device control */
    VideoDevices GetPossibleDevices();

    /* frame grabbing */
    void SetFrameDropping(bool pDrop);
    int GetCurrentFrameRef(void **pFrame, float *pFrameRate = NULL);
    void ReleaseCurrentFrameRef();
    int GetLastFrameNumber();

    VideoWidget *GetVideoWidget();

private:
    virtual void InitFrameBuffers(QString pMessage);
    virtual void DeinitFrameBuffers();
    void InitFrameBuffer(int pBufferId);
    void DoSetGrabResolution();
    virtual void DoSetCurrentDevice();
    virtual void DoPlayNewFile();
    virtual void DoSeek();
    virtual void DoSyncClock();
    virtual void DoSetFullScreenDisplay();
    virtual void HandlePlayFileError();
    virtual void HandlePlayFileSuccess();

    VideoWidget         *mVideoWidget;

    /* frame buffering */
    void                *mFrame[FRAME_BUFFER_SIZE];
    unsigned long       mFrameNumber[FRAME_BUFFER_SIZE];
    int                 mFrameSize[FRAME_BUFFER_SIZE];
    int                 mFrameCurrentIndex, mFrameGrabIndex;
    bool                mCurrentFrameRefTaken;
    int                 mDesiredResX;
    int                 mDesiredResY;
    int                 mResX;
    int                 mResY;
    int					mFrameWidthLastGrabbedFrame;
    int					mFrameHeightLastGrabbedFrame;
    int                 mPendingNewFrames;
    bool                mDropFrames;
    /* frame statistics */
    int                 mMissingFrames;
    /* A/V synch. */
    bool                mWaitForFirstFrameAfterSeeking;
    /* vide resolution detection for source */
    QTime               mTimeLastDetectedVideoResolutionChange;

    bool                mSetFullScreenDisplayAsap;
};

///////////////////////////////////////////////////////////////////////////////

}}

#endif
