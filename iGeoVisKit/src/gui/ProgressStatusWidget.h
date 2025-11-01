#ifndef _PARBAT_PROGRESS_STATUS_WIDGET_H
#define _PARBAT_PROGRESS_STATUS_WIDGET_H

#include <QWidget>
#include <QProgressBar>
#include <QLabel>
#include <QHBoxLayout>
#include <QTimer>

class ProgressStatusWidget : public QWidget
{
    Q_OBJECT

public:
    /**
        Construct a status bar progress widget.
    */
    ProgressStatusWidget(QWidget *parent = nullptr);
    
    /**
        Unallocates allocated memory
    */
    ~ProgressStatusWidget();
    
    /**
        Show the progress widget with a set number of steps to complete.
    */
    void start(int steps, bool auto_increment);
    
    /**
        Hide the progress widget and mark complete.
    */
    void end();
    
    /**
        Resets the counter to zero.
    */
    void reset();
    
    /**
        Resets the counter to zero and sets a new amount of steps to
        complete. Basically does the same thing as start() but will not show
        the progress bar if it is hidden.
    */
    void reset(int steps);
    
    /**
        Increase the number of steps that have been completed so far by one.
    */
    void increment();
    
    /**
        Increase the number of steps that have been completed so far by the
        given amount.
    */
    void increment(int steps);
    
    /**
        Returns true if the total number of steps have been completed.
    */
    bool isComplete() { return (current_steps >= total_steps); }
    
    /**
        If set to true then once the progress bar reaches the end, it will
        reset and start loading again. This allows the progress bar to show
        time passing rather than an amount of the total.
        Looping if turned off by default.
    */
    void setLooping(bool loop) { loop_at_end = loop; }

private slots:
    void autoIncrementProgress();

private:
    void setupUI();
    void init();

    QProgressBar *progressBar;
    QLabel *statusLabel;
    QTimer *autoTimer;
    
    int current_steps;
    int total_steps;
    int start_count;
    bool loop_at_end;
    bool autoIncrement;
};

#endif