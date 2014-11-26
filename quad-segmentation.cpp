 /**
 * Automatic perspective correction for quadrilateral objects. See the tutorial at
 * http://opencv-code.com/tutorials/automatic-perspective-correction-for-quadrilateral-objects/
 */
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

cv::Point2f center(0,0);

cv::Point2f computeIntersect(cv::Vec4i a, 
                             cv::Vec4i b)
{
	int x1 = a[0], y1 = a[1], x2 = a[2], y2 = a[3], x3 = b[0], y3 = b[1], x4 = b[2], y4 = b[3];
	float denom;

	if (float d = ((float)(x1 - x2) * (y3 - y4)) - ((y1 - y2) * (x3 - x4)))
	{
		cv::Point2f pt;
		pt.x = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)) / d;
		pt.y = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4)) / d;
		return pt;
	}
	else
		return cv::Point2f(-1, -1);
}

void sortCorners(std::vector<cv::Point2f>& corners, 
                 cv::Point2f center)
{
	std::vector<cv::Point2f> top, bot;

	for (int i = 0; i < corners.size(); i++)
	{
		if (corners[i].y < center.y)
			top.push_back(corners[i]);
		else
			bot.push_back(corners[i]);
	}
	corners.clear();
	
	if (top.size() == 2 && bot.size() == 2){
		cv::Point2f tl = top[0].x > top[1].x ? top[1] : top[0];
		cv::Point2f tr = top[0].x > top[1].x ? top[0] : top[1];
		cv::Point2f bl = bot[0].x > bot[1].x ? bot[1] : bot[0];
		cv::Point2f br = bot[0].x > bot[1].x ? bot[0] : bot[1];
	
		
		corners.push_back(tl);
		corners.push_back(tr);
		corners.push_back(br);
		corners.push_back(bl);
	}
}

int main()
{
    cv::Mat src = cv::imread("image2.jpg");
	if (src.empty())
		return -1;

	cv::Mat bw;
    cv::cvtColor(src, bw, CV_BGR2GRAY);
    //cv::preCornerDetect(bw, bw, 3);
    cv::blur(bw, bw, cv::Size(3, 3));
    cv::Canny(bw, bw, 100, 100, 3);

    cv::imshow("Get_the_edge_map", bw);


	std::vector<cv::Vec4i> lines;
//    cv::HoughLinesP(bw, lines, 1, CV_PI/180, 100, 70, 0);

//    std::cout<<"** lines.size() = " << lines.size() <<std::endl;

//    cv::Mat dst1 = src.clone();
//    // Draw lines
//    for (int i = 0; i < lines.size(); i++)
//    {
//        cv::Vec4i v = lines[i];
//        cv::line(dst1, cv::Point(v[0], v[1]), cv::Point(v[2], v[3]), CV_RGB(255*i/lines.size(),0,0), 3);
//        //cv::line(dst1, cv::Point(0, 0), cv::Point(150, 150), CV_RGB(0,255,0));
//    }

//    cv::imshow("dst1", dst1);
//    //cv::waitKey();

    cv::Mat dst2 = src.clone();
    std::vector<cv::Vec2f> lines_cartesian;
    cv::HoughLines(bw, lines_cartesian, 1, CV_PI/180, 100, 0, 0);
    std::cout<<"** lines_cartesian.size() = " << lines_cartesian.size() <<std::endl;

    std::vector<cv::Vec2f> resV;
    for (int i = 0; i < lines_cartesian.size(); i++)
    {
        float rho = lines_cartesian[i][0];
        float theta = lines_cartesian[i][1];

        auto it = std::find_if(resV.begin(), resV.end(), [rho, theta](cv::Vec2f&v) {
                //std::cout<<"** rho = " << rho << "\t v[0] = " << v[0] <<std::endl;
                //std::cout<<"** abs(theta - v[0]) = " << abs(theta - v[1]) << std::endl;
                return (abs(rho - v[0]) < 10) && abs(theta - v[1]) < 0.2 ;
        });
        if (it != resV.end())
            continue;

        std::cout<<"-- add Point ** rho = " << rho << "\t theta = " << theta <<std::endl;
        resV.push_back(lines_cartesian[i]);

        cv::Point pt1, pt2;
        double a = cos(theta), b =  sin(theta);
        double x0 = a*rho, y0 = b*rho;
        pt1.x = cvRound(x0 +  1000*(-b)); //the first point
        pt1.y = cvRound(y0 +  1000*(a)); //the first point
        pt2.x = cvRound(x0 -  1000*(-b)); //the second point
        pt2.y = cvRound(y0 -  1000*(a)); //the second point

        cv::line(dst2, pt1, pt2, CV_RGB(255*i/lines_cartesian.size(),0,0), 3);

        cv::Vec4i tmp(pt1.x, pt1.y, pt2.x, pt2.y);
        lines.push_back(tmp);
    }
    cv::line(dst2, cv::Point(0, 0), cv::Point(150, 150), CV_RGB(255,0,0), 3);

    cv::imshow("dst2", dst2);
    cv::waitKey();

    if(lines.size() != 4)
    {
        std::cout << "***************************************" << std::endl;
        std::cout << " lines.size() != 4 " << std::endl;
        std::cout << "***************************************" << std::endl;
        return -1;
    }


	// Expand the lines
//	for (int i = 0; i < lines.size(); i++)
//	{
//		cv::Vec4i v = lines[i];
//		lines[i][0] = 0;
//		lines[i][1] = ((float)v[1] - v[3]) / (v[0] - v[2]) * -v[0] + v[1];
//		lines[i][2] = src.cols;
//		lines[i][3] = ((float)v[1] - v[3]) / (v[0] - v[2]) * (src.cols - v[2]) + v[3];
//	}


	
	std::vector<cv::Point2f> corners;
	for (int i = 0; i < lines.size(); i++)
	{
		for (int j = i+1; j < lines.size(); j++)
		{
			cv::Point2f pt = computeIntersect(lines[i], lines[j]);
			if (pt.x >= 0 && pt.y >= 0)
				corners.push_back(pt);
		}
	}

	std::vector<cv::Point2f> approx;
	cv::approxPolyDP(cv::Mat(corners), approx, cv::arcLength(cv::Mat(corners), true) * 0.02, true);

	if (approx.size() != 4)
	{
        std::cout << "***************************************" << std::endl;
        std::cout << "The object is not quadrilateral!" << std::endl;
        std::cout << "***************************************" << std::endl;
		return -1;
	}
	
	// Get mass center
	for (int i = 0; i < corners.size(); i++)
		center += corners[i];
	center *= (1. / corners.size());

	sortCorners(corners, center);
	if (corners.size() == 0){
		std::cout << "The corners were not sorted correctly!" << std::endl;
		return -1;
	}
	cv::Mat dst = src.clone();

	// Draw lines
	for (int i = 0; i < lines.size(); i++)
	{
		cv::Vec4i v = lines[i];
		cv::line(dst, cv::Point(v[0], v[1]), cv::Point(v[2], v[3]), CV_RGB(0,255,0));
	}

	// Draw corner points
	cv::circle(dst, corners[0], 3, CV_RGB(255,0,0), 2);
	cv::circle(dst, corners[1], 3, CV_RGB(0,255,0), 2);
	cv::circle(dst, corners[2], 3, CV_RGB(0,0,255), 2);
	cv::circle(dst, corners[3], 3, CV_RGB(255,255,255), 2);

	// Draw mass center
	cv::circle(dst, center, 3, CV_RGB(255,255,0), 2);

	cv::Mat quad = cv::Mat::zeros(300, 220, CV_8UC3);

	std::vector<cv::Point2f> quad_pts;
	quad_pts.push_back(cv::Point2f(0, 0));
	quad_pts.push_back(cv::Point2f(quad.cols, 0));
	quad_pts.push_back(cv::Point2f(quad.cols, quad.rows));
	quad_pts.push_back(cv::Point2f(0, quad.rows));

	cv::Mat transmtx = cv::getPerspectiveTransform(corners, quad_pts);
	cv::warpPerspective(src, quad, transmtx, quad.size());

	cv::imshow("image", dst);
	cv::imshow("quadrilateral", quad);
	cv::waitKey();
	return 0;
}

