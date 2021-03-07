library(shinydashboard)
library(plotly)
library(lubridate)
library(dplyr)
library(shiny)

#Load the data
library(readxl)
NC_Data <- read_excel("NC_Data.xlsx")
View(NC_Data)


#Every Shiny dashboard has 3 components.
# First we build the ui - the user interface.

Categories=c("Segment", "State", "Region", "Category", "Sub-Category")

ui <- dashboardPage(
  
  title = 'Sales in North Carolina',
  
  dashboardHeader(title = 'North Carolina'),
  
  dashboardSidebar(
    menuItem('NC Sales', tabName = 'NCfig', icon = icon('line-chart'))
  ),
  
  dashboardBody(
    
    
    tabItem(tabName = 'NCfig',
            
            fluidRow(
              box(
                title = 'NC Sales',
                plotlyOutput(outputId = 'p1')),
              
              box(
                title = 'NC Profits',
                plotlyOutput(outputId = 'p2')),
              
              box(
                title = 'NC Category Sales',
                plotlyOutput(outputId = 'p3')),
              
              box(
                title = 'Select Years',
                
                sliderInput(
                  inputId='RangeYear',
                  label='Select Years to display',
                  min = unique(year(NC_Data$`Order Date`)) %>% min(),
                  max = unique(year(NC_Data$`Order Date`)) %>% max(),
                  value = c(
                    unique(year(NC_Data$`Order Date`)) %>% min(),
                    unique(year(NC_Data$`Order Date`)) %>% max()),
                  step = 1),
                
              )
              
            )
            
    )
  )
)

server <- function(input, output){
  
  
  output$p1 <- renderPlotly({
    
    theSalesChart <- NC_Data %>% 
      filter(year(NC_Data$`Order Date`) >= input$RangeYear[1] &
               year(NC_Data$`Order Date`) <= input$RangeYear[2]) %>% 
      ggplot(aes(x = year(`Order Date`), y = Sales, fill=Segment)) +
      geom_col(alpha=0.8, position = "dodge")+
      scale_fill_brewer(palette="Set1")+
      labs(
        title = paste0(input$var_y2,  "North Carolina Sales (US $) to Different Segments"), 
        x= "Consumer Type",
        y = "Sales (in $)",
        fill="Consumer Type")
    ggplotly(theSalesChart)
  })
  
  output$p2 <- renderPlotly({
    
    theProfitsChart <- NC_Data %>% 
      filter(year(NC_Data$`Order Date`) >= input$RangeYear[1] &
               year(NC_Data$`Order Date`) <= input$RangeYear[2]) %>% 
      ggplot(aes(x = year(`Order Date`), y = Profit, fill=Segment)) +
      geom_col(aes(), alpha=0.8, position = "dodge")+
      scale_fill_brewer(palette="Set1")+
      labs(
        title = paste0("North Carolina Sales (US $) to Different Segments"), 
        x= "Consumer Type",
        y = "Sales (in $)",
        fill="Consumer Type")
    
    
    ggplotly(theProfitsChart)
    
  })
  output$p3 <- renderPlotly({
    
    theCategoryChart<- NC_Data %>%
      filter(year(NC_Data$`Order Date`) >= input$RangeYear[1] &
               year(NC_Data$`Order Date`) <= input$RangeYear[2]) %>% 
      group_by(Category) %>% 
      summarise(Total_sales = max(Sales))%>% 
      ggplot(aes(x=Category,y=Total_sales,fill=Category))+
      geom_col(alpha=0.8, position = "dodge")+
      scale_fill_brewer(palette="Set1")+labs(
        title = paste0("North Carolina Sales by Category"), 
        x= "Different Category Sales",
        y = "Sales (in $)",
        fill="Category Type")
    
    
    ggplotly(theCategoryChart)
    
  })
  
}

shinyApp(ui = ui, server = server)

